#include "physics/RigidBody.h"
#include "physics/PhysicsWorld.h"
#include "core/Entity.h"
#include "core/Transform.h"
#include "core/Logger.h"
#include <physx/PxPhysicsAPI.h>

using namespace physx;
using namespace DirectX;

namespace engine::physics {

    static XMFLOAT3 toXM(const PxVec3& v) { return { v.x, v.y, v.z }; }
    static XMFLOAT4 toXM(const PxQuat& q) { return { q.x, q.y, q.z, q.w }; }

    static XMFLOAT3 getEntityPos(core::Entity* e) {
        if (auto* t = e->getComponent<core::Transform>())
            return t->getPosition();
        return { 0.f, 0.f, 0.f };
    }

    static XMFLOAT4 getEntityRot(core::Entity* e) {
        if (auto* t = e->getComponent<core::Transform>())
            return t->getRotationQuat();
        return { 0.f, 0.f, 0.f, 1.f };
    }

    RigidBody::RigidBody(PhysicsWorld* world, ColliderShape shape,
                         RigidBodyType type, float mass)
        : m_world(world), m_type(type), m_mass(mass), m_shape(shape) {}

    void RigidBody::onUpdate(float deltaTime) {
        if (!owner) return;

        if (!m_actor) {
            XMFLOAT3 pos = getEntityPos(owner);
            XMFLOAT4 rot = getEntityRot(owner);
            uint64_t id  = owner->getId();

            if (m_type == RigidBodyType::Static) {
                m_actor = m_world->createStatic(pos, rot, m_shape, id);
            } else {
                PxRigidDynamic* dyn = m_world->createDynamic(pos, rot, m_shape, m_mass, id);

                if (m_type == RigidBodyType::Kinematic)
                    dyn->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

                m_actor = dyn;
            }
        }

        if (m_type == RigidBodyType::Dynamic) {
            auto* dyn = static_cast<PxRigidDynamic*>(m_actor);
            if (dyn->isSleeping()) return;

            PxTransform t = dyn->getGlobalPose();

            if (auto* tr = owner->getComponent<core::Transform>()) {
                tr->setPosition(toXM(t.p));
                tr->setRotationQuat(toXM(t.q));
            }
        }
    }

    RigidBody::~RigidBody() {
        if (m_actor && m_world)
            m_world->removeActor(m_actor);
    }

    void RigidBody::setLinearVelocity(const XMFLOAT3& v) {
        if (auto* d = static_cast<PxRigidDynamic*>(m_actor))
            d->setLinearVelocity({ v.x, v.y, v.z });
    }

    void RigidBody::setAngularVelocity(const XMFLOAT3& v) {
        if (auto* d = static_cast<PxRigidDynamic*>(m_actor))
            d->setAngularVelocity({ v.x, v.y, v.z });
    }

    void RigidBody::addForce(const XMFLOAT3& f) {
        if (auto* d = static_cast<PxRigidDynamic*>(m_actor))
            d->addForce({ f.x, f.y, f.z });
    }

    void RigidBody::addImpulse(const XMFLOAT3& imp) {
        if (auto* d = static_cast<PxRigidDynamic*>(m_actor))
            d->addForce({ imp.x, imp.y, imp.z }, PxForceMode::eIMPULSE);
    }

    void RigidBody::setKinematicTarget(const XMFLOAT3& pos, const XMFLOAT4& rot) {
        if (auto* d = static_cast<PxRigidDynamic*>(m_actor))
            d->setKinematicTarget(PxTransform(
                PxVec3(pos.x, pos.y, pos.z),
                PxQuat(rot.x, rot.y, rot.z, rot.w)));
    }

    void RigidBody::setGravityEnabled(bool enabled) {
        if (m_actor)
            m_actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !enabled);
    }

    void RigidBody::setMass(float mass) {
        m_mass = mass;
        if (auto* d = static_cast<PxRigidDynamic*>(m_actor))
            PxRigidBodyExt::updateMassAndInertia(*d, mass);
    }

    XMFLOAT3 RigidBody::getLinearVelocity() const {
        if (auto* d = static_cast<PxRigidDynamic*>(m_actor))
            return toXM(d->getLinearVelocity());
        return { 0.f, 0.f, 0.f };
    }

    XMFLOAT3 RigidBody::getAngularVelocity() const {
        if (auto* d = static_cast<PxRigidDynamic*>(m_actor))
            return toXM(d->getAngularVelocity());
        return { 0.f, 0.f, 0.f };
    }

    void RigidBody::onCollisionEnter(uint64_t otherId) { if (onEnter) onEnter(otherId); }
    void RigidBody::onCollisionStay(uint64_t otherId)  { if (onStay)  onStay(otherId);  }
    void RigidBody::onCollisionExit(uint64_t otherId)  { if (onExit)  onExit(otherId);  }

}