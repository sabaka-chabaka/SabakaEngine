#pragma once
#include "physics/ColliderShape.h"
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <functional>
#include <cstdint>

#include <physx/PxPhysicsAPI.h>

namespace engine::physics {
    using namespace DirectX;

    struct ContactEvent {
        enum class Type { Enter, Stay, Exit };

        uint64_t entityA;
        uint64_t entityB;
        Type     type;
    };

    class PhysicsContactListener;

    class PhysicsWorld {
    public:
        PhysicsWorld();
        ~PhysicsWorld();

        PhysicsWorld(const PhysicsWorld&)            = delete;
        PhysicsWorld& operator=(const PhysicsWorld&) = delete;

        void simulate(float deltaTime);

        physx::PxRigidDynamic* createDynamic(
            const XMFLOAT3& pos,
            const XMFLOAT4& rot,
            const ColliderShape& shape,
            float mass,
            uint64_t entityId);

        physx::PxRigidStatic* createStatic(
            const XMFLOAT3& pos,
            const XMFLOAT4& rot,
            const ColliderShape&     shape,
            uint64_t                 entityId);

        void removeActor(physx::PxActor* actor);

        const std::vector<ContactEvent>& getContactEvents() const;

        physx::PxPhysics* getPhysics() const { return m_physics; }
        physx::PxScene*   getScene()   const { return m_scene;   }

    private:
        physx::PxShape* createShape(const ColliderShape& shape);

        physx::PxDefaultAllocator      m_allocator;
        physx::PxDefaultErrorCallback  m_errorCallback;
        physx::PxFoundation*           m_foundation  = nullptr;
        physx::PxPhysics*              m_physics     = nullptr;
        physx::PxDefaultCpuDispatcher* m_dispatcher  = nullptr;
        physx::PxScene*                m_scene       = nullptr;
        physx::PxPvd*                  m_pvd         = nullptr;

        std::unique_ptr<PhysicsContactListener> m_contactListener;
    };
}
