#pragma once
#include <runtime/physics/physics.hpp>
#include <runtime/subsystem.hpp>
#include <Jolt/Jolt.h>

#include <runtime/physics/components/rigid_body_component.hpp>
#include <runtime/physics/components/phys_body_component.hpp>
#include <runtime/component.hpp>
#include <runtime/entity.hpp>
#include <runtime/world.hpp>

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

class MyBodyActivationListener;
class MyContactListener;

// working name
struct PhysicsBackend: PreUpdate {
  explicit PhysicsBackend(App& app) : PreUpdate(app) {}

protected:
  auto create_phys_body_component(PhysBodyID body_id) -> PhysBodyComponent {
    return PhysBodyComponent(body_id);
  }
};

class JoltPhysicsSystem: public PhysicsBackend {
public:
  JoltPhysicsSystem(App& app);

public:
  void init() override;
  void shutdown() override;
  void update(float dt) override {}
  void fixed_update(float dt) override;

private:

private:
  auto get_motion(PhysMotionType motion) -> JPH::EMotionType;

  void on_add_rigid_body(OnAdd<RigidBodyComponent> event);

  void add_bodies_from_list();

  void sync_transforms_with_physics();

private:
  class ObjectLayerPairFilterImpl;
  class BPLayerInterfaceImpl;
  class ObjectVsBroadPhaseLayerFilterImpl;

//   /// Class that determines if two object layers can collide
//   class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
//   {
//   public:
//     bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
//     {
//       switch (inObject1)
//       {
//       case Layers::NON_MOVING:
//         return inObject2 == Layers::MOVING; // Non moving only collides with moving
//       case Layers::MOVING:
//         return true; // Moving collides with everything
//       default:
//         JPH_ASSERT(false);
//         return false;
//       }
//     }
//   };

//   // BroadPhaseLayerInterface implementation
//   // This defines a mapping between object and broadphase layers.
//   class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
//   {
//   public:
//     BPLayerInterfaceImpl()
//     {
//       // Create a mapping table from object to broad phase layer
//       mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
//       mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
//     }

//     virtual uint					GetNumBroadPhaseLayers() const override
//     {
//       return BroadPhaseLayers::NUM_LAYERS;
//     }

//     virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
//     {
//       JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
//       return mObjectToBroadPhase[inLayer];
//     }

// #if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
//     virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
//     {
//       switch ((BroadPhaseLayer::Type)inLayer)
//       {
//       case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
//       case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
//       default:													JPH_ASSERT(false); return "INVALID";
//       }
//     }
// #endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

//   private:
//     BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
//   };

//   /// Class that determines if an object layer can collide with a broadphase layer
//   class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
//   {
//   public:
//     virtual bool				ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
//     {
//       switch (inLayer1)
//       {
//       case Layers::NON_MOVING:
//         return inLayer2 == BroadPhaseLayers::MOVING;
//       case Layers::MOVING:
//         return true;
//       default:
//         JPH_ASSERT(false);
//         return false;
//       }
//     }
//   };

private:
  JPH::PhysicsSystem physics;
  JPH::TempAllocatorImpl* temp_alloc{};
  JPH::JobSystemThreadPool* thread_pool{};

  // Create mapping table from object layer to broadphase layer
    // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
    // Also have a look at BroadPhaseLayerInterfaceTable or BroadPhaseLayerInterfaceMask for a simpler interface.
  Unique<BPLayerInterfaceImpl> broad_phase_layer_interface;

  // Create class that filters object vs broadphase layers
  // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
  // Also have a look at ObjectVsBroadPhaseLayerFilterTable or ObjectVsBroadPhaseLayerFilterMask for a simpler interface.
  Unique<ObjectVsBroadPhaseLayerFilterImpl> object_vs_broadphase_layer_filter;

  // Create class that filters object vs object layers
  // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
  // Also have a look at ObjectLayerPairFilterTable or ObjectLayerPairFilterMask for a simpler interface.
  Unique<ObjectLayerPairFilterImpl> object_vs_object_layer_filter;

  Unique<MyBodyActivationListener> body_activation_listener;
  Unique<MyContactListener> contact_listener;

  JPH::BodyInterface* body_interface{};
private:
  u32 step = 0;

  struct BodyAddInfo {
    Entity entity;
    JPH::EMotionType motion;
    JPH::EBodyType body_type;
  };

  Vec<BodyAddInfo> body_add_list;
  Vec<Entity> body_remove_list;

  HashMap<EntityID, JPH::BodyID> entity_to_body_map;
  HashMap<JPH::BodyID, EntityID> body_to_entity_map;
};