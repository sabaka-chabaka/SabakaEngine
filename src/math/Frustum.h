#pragma once
#include "AABB.h"
#include <DirectXMath.h>

namespace engine::math {
    using namespace DirectX;

    struct Plane {
        float a, b, c, d;
    };

    class Frustum {
    public:
        void buildFromViewProjection(CXMMATRIX viewProjection);

        bool intersects(const AABB& aabb);

    private:
        Plane m_planes[6];
    };
}
