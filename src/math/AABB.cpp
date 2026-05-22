#define NOMINMAX
#include "math/AABB.h"
#include <algorithm>

namespace engine::math {

    AABB AABB::fromVertices(const std::vector<renderer::Vertex>& vertices) {
        AABB box;
        for (auto& v : vertices) {
            box.min.x = std::min(box.min.x, v.x);
            box.min.y = std::min(box.min.y, v.y);
            box.min.z = std::min(box.min.z, v.z);
            box.max.x = std::max(box.max.x, v.x);
            box.max.y = std::max(box.max.y, v.y);
            box.max.z = std::max(box.max.z, v.z);
        }

        return box;
    }

    AABB AABB::transform(CXMMATRIX matrix) const {
        XMFLOAT3 corners[8] = {
            { min.x, min.y, min.z },
            { max.x, min.y, min.z },
            { min.x, max.y, min.z },
            { max.x, max.y, min.z },
            { min.x, min.y, max.z },
            { max.x, min.y, max.z },
            { min.x, max.y, max.z },
            { max.x, max.y, max.z },
        };

        AABB result;
        for (auto& c : corners) {
            XMVECTOR p = XMVector3TransformCoord(XMLoadFloat3(&c), matrix);
            XMFLOAT3 t;
            XMStoreFloat3(&t, p);
            result.min.x = std::min(result.min.x, t.x);
            result.min.y = std::min(result.min.y, t.y);
            result.min.z = std::min(result.min.z, t.z);
            result.max.x = std::max(result.max.x, t.x);
            result.max.y = std::max(result.max.y, t.y);
            result.max.z = std::max(result.max.z, t.z);
        }
        return result;
    }

    XMFLOAT3 AABB::center() const {
        return {
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f,
            (min.z + max.z) * 0.5f,
        };
    }

    XMFLOAT3 AABB::extents() const {
        return {
            (max.x - min.x) * 0.5f,
            (max.y - min.y) * 0.5f,
            (max.z - min.z) * 0.5f,
        };
    }

    bool AABB::contains(const XMFLOAT3& p) const {
        return p.x >= min.x && p.x <= max.x
            && p.y >= min.y && p.y <= max.y
            && p.z >= min.z && p.z <= max.z;
    }

    bool AABB::intersects(const AABB& o) const {
        return min.x <= o.max.x && max.x >= o.min.x
            && min.y <= o.max.y && max.y >= o.min.y
            && min.z <= o.max.z && max.z >= o.min.z;
    }

    bool AABB::isValid() const {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }
}