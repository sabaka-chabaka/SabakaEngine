#pragma once
#include "core/Component.h"
#include "math/AABB.h"

namespace engine::core {

    class BoundingBoxComponent : public Component {
    public:
        BoundingBoxComponent() = default;
        ~BoundingBoxComponent() override;

        void setLocalAABB(const math::AABB& aabb);
        void buildFromVertices(const std::vector<renderer::Vertex>& vertices);

        const math::AABB& getLocalAABB() const;
        math::AABB        getWorldAABB() const;

    private:
        math::AABB m_localAABB;
    };
}