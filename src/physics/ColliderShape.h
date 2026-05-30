#pragma once
#include <DirectXMath.h>

namespace engine::physics {
    using namespace DirectX;

    enum class ColliderType {
        Box,
        Sphere,
        Capsule,
    };

    struct BoxCollider {
        XMFLOAT3 HalfExtents = {0.5f,0.5f,0.5f};
    };

    struct SphereCollider {
        float radius = 0.5f;
    };

    struct CapsuleCollider {
        float radius = 0.25f;
        float halfHeight = 0.5f;
    };

    struct ColliderShape {
        ColliderType type = ColliderType::Box;

        union {
            BoxCollider box;
            SphereCollider sphere;
            CapsuleCollider capsule;
        };

        float staticFriction = 0.5f;
        float dynamicFriction = 0.5f;
        float restitution = 0.3f;

        XMFLOAT3 localOffset   = {0.f,0.f,0.f};
        XMFLOAT4 localRotation = {0.f,0.f,0.f,1.f};

        ColliderShape() : box{} {}

        static ColliderShape makeBox(DirectX::XMFLOAT3 halfExtents) {
            ColliderShape s;
            s.type            = ColliderType::Box;
            s.box.halfExtents = halfExtents;
            return s;
        }

        static ColliderShape makeSphere(float radius) {
            ColliderShape s;
            s.type          = ColliderType::Sphere;
            s.sphere.radius = radius;
            return s;
        }

        static ColliderShape makeCapsule(float radius, float halfHeight) {
            ColliderShape s;
            s.type                = ColliderType::Capsule;
            s.capsule.radius      = radius;
            s.capsule.halfHeight  = halfHeight;
            return s;
        }
    };
}