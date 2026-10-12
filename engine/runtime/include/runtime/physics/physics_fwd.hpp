#pragma once
#include <core/def.hpp>

enum struct PhysBodyID: u32;
enum struct PhysMotionType: u8;

struct RigidBodyComponent;
struct SoftBodyComponent;
struct BoxColliderComponent;
struct SphereColliderComponent;
struct CapsuleColliderComponent;
struct PhysicsBackend;