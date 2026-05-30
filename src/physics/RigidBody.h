#pragma once
#include "core/Component.h"
#include "physics/ColliderShape.h"
#include <DirectXMath.h>
#include <functional>

namespace physx {
    class PxRigidActor;
    class PxRigidDynamic;
}

namespace engine::physics {
    using namespace DirectX;

    class PhysicsWorld;

    enum class RigidBodyType {
        Static,
        Dynamic,
        Kinematic
    };

    using CollisionCallback = std::function<void(uint64_t otherEntityId)>;

    class RigidBody : public core::Component {
    public:
        RigidBody(PhysicsWorld* world, ColliderShape shape,
                  RigidBodyType type = RigidBodyType::Dynamic, float mass = 1.f);

        ~RigidBody() override;

        void onUpdate(float deltaTime) override;

        void setLinearVelocity(const XMFLOAT3 &v);

        void setAngularVelocity(const XMFLOAT3 &v);

        void addForce(const XMFLOAT3 &force);

        void addImpulse(const XMFLOAT3 &impulse);

        void setKinematicTarget(const XMFLOAT3 &pos, const XMFLOAT4 &rot);

        void setGravityEnabled(bool enabled);

        void setMass(float mass);

        XMFLOAT3 getLinearVelocity() const;

        XMFLOAT3 getAngularVelocity() const;

        RigidBodyType getType() const { return m_type; }
        physx::PxRigidActor* getActor() const { return m_actor; }

        void onCollisionEnter(uint64_t otherId);

        void onCollisionStay(uint64_t otherId);

        void onCollisionExit(uint64_t otherId);

        CollisionCallback onEnter;
        CollisionCallback onStay;
        CollisionCallback onExit;

    private:
        PhysicsWorld* m_world = nullptr;
        physx::PxRigidActor* m_actor = nullptr;
        RigidBodyType m_type;
        float m_mass;
        ColliderShape m_shape;
    };
}