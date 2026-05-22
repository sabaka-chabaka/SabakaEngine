#include "Frustum.h"

namespace engine::math{
    static Plane normalizePlane(float a, float b, float c, float d) {
        float len = sqrtf(a * a + b * b + c * c);
        if (len < 1e-8f) return { a, b, c, d };
        return { a / len, b / len, c / len, d / len };
    }

    void Frustum::buildFromViewProjection(CXMMATRIX vp) {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, vp);

        m_planes[0] = normalizePlane(m._14 + m._11, m._24 + m._21, m._34 + m._31, m._44 + m._41);
        m_planes[1] = normalizePlane(m._14 - m._11, m._24 - m._21, m._34 - m._31, m._44 - m._41);
        m_planes[2] = normalizePlane(m._14 - m._12, m._24 - m._22, m._34 - m._32, m._44 - m._42);
        m_planes[3] = normalizePlane(m._14 + m._12, m._24 + m._22, m._34 + m._32, m._44 + m._42);
        m_planes[4] = normalizePlane(m._13,          m._23,          m._33,          m._43        );
        m_planes[5] = normalizePlane(m._14 - m._13, m._24 - m._23, m._34 - m._33, m._44 - m._43);
    }

    bool Frustum::intersects(const AABB &aabb) {
        for (auto& p : m_planes) {
            float px = p.a >= 0.0f ? aabb.max.x : aabb.min.x;
            float py = p.b >= 0.0f ? aabb.max.y : aabb.min.y;
            float pz = p.c >= 0.0f ? aabb.max.z : aabb.min.z;

            if (p.a * px + p.b * py + p.c * pz + p.d < 0.0f)
                return false;
        }
        return true;
    }
}
