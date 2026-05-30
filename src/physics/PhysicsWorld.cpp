#include "PhysicsWorld.h"
#include "core/Logger.h"
#include <stdexcept>

namespace engine::physics {
    using namespace physx;
    using namespace DirectX;

    static PxTransform toPxTransform(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT4& rot) {
        return PxTransform(
            PxVec3(pos.x, pos.y, pos.z),
            PxQuat(rot.x, rot.y, rot.z, rot.w));
    }

    class PhysicsContactListener : public PxSimulationEventCallback {
    public:
        void onContact(const PxContactPairHeader& header,
                       const PxContactPair* pairs, PxU32 nbPairs) override
        {
            uint64_t idA = reinterpret_cast<uint64_t>(header.actors[0]->userData);
            uint64_t idB = reinterpret_cast<uint64_t>(header.actors[1]->userData);

            for (PxU32 i = 0; i < nbPairs; ++i) {
                const PxContactPair& pair = pairs[i];

                ContactEvent::Type type;
                if (pair.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
                    type = ContactEvent::Type::Enter;
                else if (pair.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
                    type = ContactEvent::Type::Stay;
                else if (pair.events & PxPairFlag::eNOTIFY_TOUCH_LOST)
                    type = ContactEvent::Type::Exit;
                else
                    continue;

                m_events.push_back({ idA, idB, type });
            }
        }

        void onTrigger(PxTriggerPair*, PxU32) override {}
        void onWake(PxActor**, PxU32)         override {}
        void onSleep(PxActor**, PxU32)        override {}
        void onAdvance(const PxRigidBody* const*, const PxTransform*, PxU32) override {}
        void onConstraintBreak(PxConstraintInfo*, PxU32) override {}

        std::vector<ContactEvent> m_events;
    };

    PhysicsWorld::PhysicsWorld() {
        m_foundation = PxCreateFoundation(
            PX_PHYSICS_VERSION, m_allocator, m_errorCallback);

        if (!m_foundation)
            throw std::runtime_error("PhysicsWorld: PxCreateFoundation failed");

        m_physics = PxCreatePhysics(
            PX_PHYSICS_VERSION, *m_foundation, PxTolerancesScale(), true, nullptr);

        if (!m_physics)
            throw std::runtime_error("PhysicsWorld: PxCreatePhysics failed");

        m_dispatcher = PxDefaultCpuDispatcherCreate(2);

        PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
        sceneDesc.gravity        = PxVec3(0.f, -9.81f, 0.f);
        sceneDesc.cpuDispatcher  = m_dispatcher;
        sceneDesc.filterShader   = PxDefaultSimulationFilterShader;
        sceneDesc.flags         |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;

        m_contactListener = std::make_unique<PhysicsContactListener>();
        sceneDesc.simulationEventCallback = m_contactListener.get();

        m_scene = m_physics->createScene(sceneDesc);

        if (!m_scene)
            throw std::runtime_error("PhysicsWorld: createScene failed");

        LOG_INFO("PhysicsWorld: PhysX initialized");
    }

    PhysicsWorld::~PhysicsWorld() {
        if (m_scene)     { m_scene->release();     m_scene     = nullptr; }
        if (m_dispatcher){ m_dispatcher->release(); m_dispatcher= nullptr; }
        if (m_physics)   { m_physics->release();   m_physics   = nullptr; }
        if (m_foundation){ m_foundation->release(); m_foundation= nullptr; }
    }

    void PhysicsWorld::simulate(float deltaTime) {
        m_contactListener->m_events.clear();

        constexpr float MIN_DT = 1.f / 240.f;
        if (deltaTime < MIN_DT) deltaTime = MIN_DT;

        m_scene->simulate(deltaTime);
        m_scene->fetchResults(true);
    }

    PxShape* PhysicsWorld::createShape(const ColliderShape& shape) {
        PxMaterial* mat = m_physics->createMaterial(
            shape.staticFriction, shape.dynamicFriction, shape.restitution);

        PxShape* px = nullptr;

        switch (shape.type) {
        case ColliderType::Box: {
            auto& b = shape.box;
            px = m_physics->createShape(
                PxBoxGeometry(b.halfExtents.x, b.halfExtents.y, b.halfExtents.z), *mat);
            break;
        }
        case ColliderType::Sphere:
            px = m_physics->createShape(
                PxSphereGeometry(shape.sphere.radius), *mat);
            break;

        case ColliderType::Capsule:
            px = m_physics->createShape(
                PxCapsuleGeometry(shape.capsule.radius, shape.capsule.halfHeight), *mat);
            break;
        }

        mat->release();

        PxTransform localPose(
            PxVec3(shape.localOffset.x, shape.localOffset.y, shape.localOffset.z),
            PxQuat(shape.localRotation.x, shape.localRotation.y,
                   shape.localRotation.z, shape.localRotation.w));
        px->setLocalPose(localPose);

        return px;
    }

    PxRigidDynamic* PhysicsWorld::createDynamic(
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT4& rot,
        const ColliderShape&     shape,
        float                    mass,
        uint64_t                 entityId)
    {
        PxRigidDynamic* body = m_physics->createRigidDynamic(toPxTransform(pos, rot));
        body->userData = reinterpret_cast<void*>(entityId);

        PxShape* s = createShape(shape);
        body->attachShape(*s);
        s->release();

        PxRigidBodyExt::updateMassAndInertia(*body, mass);

        m_scene->addActor(*body);
        return body;
    }

    PxRigidStatic* PhysicsWorld::createStatic(
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT4& rot,
        const ColliderShape&     shape,
        uint64_t                 entityId)
    {
        PxRigidStatic* body = m_physics->createRigidStatic(toPxTransform(pos, rot));
        body->userData = reinterpret_cast<void*>(entityId);

        PxShape* s = createShape(shape);
        body->attachShape(*s);
        s->release();

        m_scene->addActor(*body);
        return body;
    }

    void PhysicsWorld::removeActor(PxActor* actor) {
        if (actor) m_scene->removeActor(*actor);
    }

    const std::vector<ContactEvent>& PhysicsWorld::getContactEvents() const {
        return m_contactListener->m_events;
    }
}
