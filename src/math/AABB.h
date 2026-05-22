#pragma once
#include "renderer/Mesh.h"
#include <DirectXMath.h>
#include <vector>

namespace engine::math {
    using namespace DirectX;

    struct AABB {
        XMFLOAT3 min = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
        XMFLOAT3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

        static AABB fromVertices(const std::vector<renderer::Vertex>& vertices);

        AABB transform(CXMMATRIX matrix) const;

        XMFLOAT3 center()  const;
        XMFLOAT3 extents() const;

        bool contains(const XMFLOAT3& point) const;
        bool intersects(const AABB& other)   const;
        bool isValid()                        const;
    };
}