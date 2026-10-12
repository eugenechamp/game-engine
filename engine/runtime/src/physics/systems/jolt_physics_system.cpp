#include <runtime/physics/systems/jolt_physics_system.hpp>

#include <runtime/entity.hpp>
// #include <runtime/world.hpp>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <runtime/components/position.hpp>
#include <runtime/components/rotation.hpp>
#include <runtime/components/scale.hpp>
#include <runtime/components/velocity.hpp>

#include <runtime/physics/components/phys_body_component.hpp> //

static void TraceImpl(const char* inFMT, ...)
{
  // Format the message
  va_list list;
  va_start(list, inFMT);
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), inFMT, list);
  va_end(list);

  // Print to the TTY
  std::cout << buffer << std::endl;
}

using namespace JPH;
using namespace JPH::literals;

using namespace std;
// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
  static constexpr BroadPhaseLayer NON_MOVING(0);
  static constexpr BroadPhaseLayer MOVING(1);
  static constexpr uint NUM_LAYERS(2);
};

namespace Layers
{
  static constexpr ObjectLayer NON_MOVING = 0;
  static constexpr ObjectLayer MOVING = 1;
  static constexpr ObjectLayer NUM_LAYERS = 2;
};

// An example contact listener
class MyContactListener : public ContactListener
{
public:
  // See: ContactListener
  virtual ValidateResult	OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override
  {
    cout << "Contact validate callback" << endl;

    // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
    return ValidateResult::AcceptAllContactsForThisBodyPair;
  }

  virtual void			OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
  {
    cout << "A contact was added" << endl;
  }

  virtual void			OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
  {
    cout << "A contact was persisted" << endl;
  }

  virtual void			OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
  {
    cout << "A contact was removed" << endl;
  }
};

// An example activation listener
class MyBodyActivationListener : public BodyActivationListener
{
public:
  virtual void		OnBodyActivated(const BodyID& inBodyID, uint64 inBodyUserData) override
  {
    cout << "A body got activated" << endl;
  }

  virtual void		OnBodyDeactivated(const BodyID& inBodyID, uint64 inBodyUserData) override
  {
    cout << "A body went to sleep" << endl;
  }
};

namespace {
  void add_collider(Entity entity) {

  }
}

/// Class that determines if two object layers can collide
class JoltPhysicsSystem::ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
  bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
  {
    switch (inObject1)
    {
    case Layers::NON_MOVING:
      return inObject2 == Layers::MOVING; // Non moving only collides with moving
    case Layers::MOVING:
      return true; // Moving collides with everything
    default:
      JPH_ASSERT(false);
      return false;
    }
  }
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class JoltPhysicsSystem::BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
  BPLayerInterfaceImpl()
  {
    // Create a mapping table from object to broad phase layer
    mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
    mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
  }

  virtual uint					GetNumBroadPhaseLayers() const override
  {
    return BroadPhaseLayers::NUM_LAYERS;
  }

  virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
  {
    JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
    return mObjectToBroadPhase[inLayer];
  }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
  {
    switch ((BroadPhaseLayer::Type)inLayer)
    {
    case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
    case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
    default:													JPH_ASSERT(false); return "INVALID";
    }
  }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
  BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class JoltPhysicsSystem::ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
  virtual bool				ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
  {
    switch (inLayer1)
    {
    case Layers::NON_MOVING:
      return inLayer2 == BroadPhaseLayers::MOVING;
    case Layers::MOVING:
      return true;
    default:
      JPH_ASSERT(false);
      return false;
    }
  }
};

JoltPhysicsSystem::JoltPhysicsSystem(App &app): PhysicsBackend(app) {
}

void JoltPhysicsSystem::init() {
  JPH::RegisterDefaultAllocator();
  JPH::Trace = TraceImpl;
  JPH::Factory::sInstance = new Factory();
  JPH::RegisterTypes();
  
  temp_alloc  = new TempAllocatorImpl(10 * 1024 * 1024);
  thread_pool = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
  
  broad_phase_layer_interface = Unique<BPLayerInterfaceImpl>(new BPLayerInterfaceImpl);
  object_vs_broadphase_layer_filter = Unique<ObjectVsBroadPhaseLayerFilterImpl>(new ObjectVsBroadPhaseLayerFilterImpl);
  object_vs_object_layer_filter = Unique<ObjectLayerPairFilterImpl>(new ObjectLayerPairFilterImpl);

  this->body_activation_listener = Unique<MyBodyActivationListener>(new MyBodyActivationListener);
  this->contact_listener = Unique<MyContactListener>(new MyContactListener);
  
  // Create class that filters object vs object layers
  // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
  // Also have a look at ObjectLayerPairFilterTable or ObjectLayerPairFilterMask for a simpler interface.

  const u32 cMaxBodies = 1024;

  // This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
  const u32 cNumBodyMutexes = 0;
  // This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
  // body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
  // too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
  // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
  const uint cMaxBodyPairs = 1024;

  // This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
  // number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
  // Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
  const uint cMaxContactConstraints = 1024;

  physics.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *broad_phase_layer_interface, *object_vs_broadphase_layer_filter, *object_vs_object_layer_filter);
  physics.SetBodyActivationListener(body_activation_listener.get());
  physics.SetContactListener(contact_listener.get());

  body_interface = &physics.GetBodyInterface();

  //
  //floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.
  //
  //ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
  //ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()
  //
  //BodyCreationSettings floor_settings(floor_shape, RVec3(0.0_r, -1.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

  //Body* floor = body_interface->CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr
  //body_interface->AddBody(floor->GetID(), EActivation::DontActivate);
  
  //float scale = 100.f;

  /*Body& floor = *body_interface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(1.0f, 0.1f, 1.0f)), RVec3(Vec3(1.f, 0.0f, 0.0f)), Quat::sIdentity(), EMotionType::Static, Layers::MOVING));
  body_interface->AddBody(floor.GetID(), EActivation::Activate);*/

  // body_interface->SetPosition(floor.GetID(), Vec3(), EActivation::Activate);

  //BodyCreationSettings sphere_settings(new SphereShape(0.5f), RVec3(0.0_r, 2.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
  //sphere_id = body_interface->CreateAndAddBody(sphere_settings, EActivation::Activate);

  //// Now you can interact with the dynamic body, in this case we're going to give it a velocity.
  //// (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
  //body_interface->SetLinearVelocity(sphere_id, Vec3(0.0f, -5.0f, 0.0f));
  
  events().listen(&JoltPhysicsSystem::on_add_rigid_body, this);
  
  //events().listen(&JoltPhysicsSystem::remove_physics_body, this);

  // world().observer(Fn{ [this](OnAdd<PhysicsBodyComponent> event) { on_add_physics_body(event); } });
  // world().observer(Fn{ [this](OnRemove<PhysicsBodyComponent> event) { remove_physics_body(event); } });
}

void JoltPhysicsSystem::shutdown() {
}

void JoltPhysicsSystem::fixed_update(float dt) {
  ++step;
  
  add_bodies_from_list();

  // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
  const int cCollisionSteps = 1;
  
  // sync with game state

  // Step the world
  physics.Update(dt, cCollisionSteps, temp_alloc, thread_pool);
  sync_transforms_with_physics();
}

auto JoltPhysicsSystem::get_motion(PhysMotionType motion) -> JPH::EMotionType {
  switch (motion) {
    case PhysMotionType::Dynamic: return JPH::EMotionType::Dynamic;
    case PhysMotionType::Static: return JPH::EMotionType::Static;
    case PhysMotionType::Kinematic: return JPH::EMotionType::Kinematic;
  };
  return {};
}

void JoltPhysicsSystem::on_add_rigid_body(OnAdd<RigidBodyComponent> event) {
  Entity entity = event.entity;
  
  if(entity.has<PhysBodyComponent>()) // entity has physics body
  {
    // PhysBodyComponent phys_body = entity.get<PhysBodyComponent>();
    // get_jolt_body(phys_body.body);
  }
  else
  {
    auto motion = get_motion(entity.get<RigidBodyComponent>().motion);
    
    JPH::BodyCreationSettings settings{ new BoxShape(Vec3(1.f, 1.0f, 1.f)), RVec3(0.f, 0.f, 0.f), Quat::sIdentity(), motion, Layers::NON_MOVING };
    settings.mMotionType = motion;

    Body* body = body_interface->CreateBody(settings); // memory leak. add body to BodyAddInfo
    body->SetUserData(u64(entity.id()));
    entity.add(create_phys_body_component(PhysBodyID{ body->GetID().GetIndex() }));
    
    body_add_list.push_back(BodyAddInfo {
      .entity = entity,
      .motion = motion,
      .body_type = EBodyType::RigidBody,
    });
  }
}

void JoltPhysicsSystem::add_bodies_from_list() {
  for (auto& [entity, motion, body_type] : body_add_list)
  {
    BoxShapeSettings box_settings;
    BodyCreationSettings settings(new BoxShape(Vec3(1.f, 0.1f, 1.f)), RVec3(0.0_r, 0.0_r, 0.0_r), Quat::sIdentity(), motion, Layers::MOVING);
    // BodyCreationSettings settings(new SphereShape(2.f), RVec3(0.0_r, 2.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);

    if (entity.has<Position>())
    {
      auto& pos = entity.get<Position>();
      settings.mPosition = RVec3(pos.x, pos.y, pos.z);
    }
    if (entity.has<Rotation>())
    {
      auto& component = entity.get<Rotation>();
      auto& rotation  = component.quat;
      settings.mRotation = Quat(rotation.x, rotation.y, rotation.z, rotation.w);
    }

    Body* body = body_interface->CreateBody(settings);
    body_interface->AddBody(body->GetID(), EActivation::Activate);
    
    entity_to_body_map.emplace(pair{ entity.id(), body->GetID() });
  }
  body_add_list.clear();
}

void JoltPhysicsSystem::sync_transforms_with_physics() {
  for (auto& [entity, body_id] : entity_to_body_map)
  {
    Position* position = world().get_mut_component<Position>(entity);
    Rotation* rotation = world().get_mut_component<Rotation>(entity);

    static_assert(sizeof(mat4) == sizeof(Mat44));

#if 0
    auto trans = body_interface->GetWorldTransform(body_id);

    mat4 model;
    vec3 scale;
    vec3 translation;
    vec3 skew;
    vec4 perspective;

    std::memcpy(&model, &trans, sizeof(mat4));

    glm::decompose(model, scale, rotation->quat, translation, skew, perspective);
#endif // 0

    auto q = body_interface->GetRotation(body_id);

    rotation->quat = quat::wxyz(q.GetW(), q.GetX(), q.GetY(), q.GetZ());

    Vec3 pos_vec = body_interface->GetPosition(body_id);
    *position = Position{ pos_vec.GetX(), pos_vec.GetY(), pos_vec.GetZ() };
  }
}