#include "core/BoundingBoxComponent.h"
#include "core/Entity.h"
#include "core/Transform.h"
#include "core/SceneNode.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace engine::core {

    BoundingBoxComponent::~BoundingBoxComponent() = default;

    void BoundingBoxComponent::setLocalAABB(const math::AABB& aabb) {
        m_localAABB = aabb;
    }

    void BoundingBoxComponent::buildFromVertices(const std::vector<renderer::Vertex>& vertices) {
        m_localAABB = math::AABB::fromVertices(vertices);
    }

    const math::AABB& BoundingBoxComponent::getLocalAABB() const {
        return m_localAABB;
    }

    math::AABB BoundingBoxComponent::getWorldAABB() const {
        if (!owner) return m_localAABB;

        XMMATRIX world = XMMatrixIdentity();

        if (auto* node = owner->getComponent<SceneNode>()) {
            world = node->getWorldMatrix();
        } else if (auto* transform = owner->getComponent<Transform>()) {
            world = transform->getWorldMatrix();
        }

        return m_localAABB.transform(world);
    }
}