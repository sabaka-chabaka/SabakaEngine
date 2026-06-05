#define NOMINMAX
#include "editor/GizmoController.h"
#include "core/Entity.h"
#include "core/Transform.h"
#include "renderer/Camera.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace DirectX;

namespace engine::editor {

    static constexpr float kPickCylRadius = 0.12f;
    static constexpr float kPickAxisLen   = 1.0f;
    static constexpr float kPickCircleR   = 1.0f;
    static constexpr float kPickCircleTol = 0.18f;
    static constexpr float kPickQuadSize  = 0.28f;

    void GizmoController::setEntity(core::Entity* entity) {
        m_entity   = entity;
        m_dragging = false;
        m_hoveredAxis = GizmoAxis::None;
    }

    GizmoController::Ray GizmoController::buildRay(int mx, int my,
                                                    int vw, int vh,
                                                    renderer::Camera* cam) const
    {
        float ndcX = (2.f * mx / vw) - 1.f;
        float ndcY = 1.f - (2.f * my / vh);

        XMMATRIX proj    = cam->getProjectionMatrix();
        XMMATRIX view    = cam->getViewMatrix();
        XMMATRIX invView = XMMatrixInverse(nullptr, view);
        XMMATRIX invProj = XMMatrixInverse(nullptr, proj);

        XMVECTOR clipNear = XMVectorSet(ndcX, ndcY, 0.f, 1.f);
        XMVECTOR clipFar  = XMVectorSet(ndcX, ndcY, 1.f, 1.f);

        auto unproject = [&](XMVECTOR clip) {
            XMVECTOR v = XMVector4Transform(clip, invProj);
            v = XMVectorDivide(v, XMVectorSplatW(v));
            v = XMVector4Transform(v, invView);
            return v;
        };

        XMVECTOR wNear = unproject(clipNear);
        XMVECTOR wFar  = unproject(clipFar);

        Ray ray;
        XMStoreFloat3(&ray.origin, wNear);
        XMStoreFloat3(&ray.dir,
            XMVector3Normalize(XMVectorSubtract(wFar, wNear)));
        return ray;
    }

    bool GizmoController::rayPlane(const Ray& ray,
                                    const XMFLOAT3& planeN,
                                    const XMFLOAT3& planeP,
                                    float& t) const
    {
        XMVECTOR n   = XMVector3Normalize(XMLoadFloat3(&planeN));
        XMVECTOR d   = XMLoadFloat3(&ray.dir);
        float    dnd = XMVectorGetX(XMVector3Dot(d, n));
        if (fabsf(dnd) < 1e-6f) return false;
        XMVECTOR op  = XMVectorSubtract(XMLoadFloat3(&planeP), XMLoadFloat3(&ray.origin));
        t = XMVectorGetX(XMVector3Dot(op, n)) / dnd;
        return t >= 0.f;
    }

    bool GizmoController::rayCylinder(const Ray& ray,
                                       const XMFLOAT3& axisDir,
                                       const XMFLOAT3& origin,
                                       float length, float radius) const
    {
        XMVECTOR d = XMLoadFloat3(&ray.dir);
        XMVECTOR o = XMVectorSubtract(XMLoadFloat3(&ray.origin), XMLoadFloat3(&origin));
        XMVECTOR a = XMVector3Normalize(XMLoadFloat3(&axisDir));

        XMVECTOR dPerp = XMVectorSubtract(d, XMVectorScale(a, XMVectorGetX(XMVector3Dot(d, a))));
        XMVECTOR oPerp = XMVectorSubtract(o, XMVectorScale(a, XMVectorGetX(XMVector3Dot(o, a))));

        float A = XMVectorGetX(XMVector3Dot(dPerp, dPerp));
        if (fabsf(A) < 1e-9f) return false;
        float B = 2.f * XMVectorGetX(XMVector3Dot(dPerp, oPerp));
        float C = XMVectorGetX(XMVector3Dot(oPerp, oPerp)) - radius * radius;

        float disc = B * B - 4.f * A * C;
        if (disc < 0.f) return false;

        float t = (-B - sqrtf(disc)) / (2.f * A);
        if (t < 0.f) t = (-B + sqrtf(disc)) / (2.f * A);
        if (t < 0.f) return false;

        XMVECTOR hit = XMVectorAdd(XMLoadFloat3(&ray.origin),
                                   XMVectorScale(XMLoadFloat3(&ray.dir), t));
        XMVECTOR local = XMVectorSubtract(hit, XMLoadFloat3(&origin));
        float    proj  = XMVectorGetX(XMVector3Dot(local, a));
        return proj >= 0.f && proj <= length;
    }

    bool GizmoController::rayDisk(const Ray& ray,
                                   const XMFLOAT3& normal,
                                   const XMFLOAT3& center,
                                   float innerR, float outerR) const
    {
        float t;
        if (!rayPlane(ray, normal, center, t)) return false;

        XMVECTOR hit = XMVectorAdd(XMLoadFloat3(&ray.origin),
                                   XMVectorScale(XMLoadFloat3(&ray.dir), t));
        float dist = XMVectorGetX(XMVector3Length(
            XMVectorSubtract(hit, XMLoadFloat3(&center))));
        return dist >= innerR && dist <= outerR;
    }

    GizmoAxis GizmoController::pickAxis(const Ray& ray,
                                         const XMFLOAT3& gizmoPos,
                                         float scale) const
    {
        float r  = kPickCylRadius * scale;
        float L  = kPickAxisLen   * scale;
        float cr = kPickCircleR   * scale;
        float ct = kPickCircleTol * scale;
        float qs = kPickQuadSize  * scale;

        if (m_mode == GizmoMode::Translate) {
            if (rayCylinder(ray, {1,0,0}, gizmoPos, L, r)) return GizmoAxis::X;
            if (rayCylinder(ray, {0,1,0}, gizmoPos, L, r)) return GizmoAxis::Y;
            if (rayCylinder(ray, {0,0,1}, gizmoPos, L, r)) return GizmoAxis::Z;

            XMFLOAT3 xy = { gizmoPos.x + qs*0.5f, gizmoPos.y + qs*0.5f, gizmoPos.z };
            XMFLOAT3 xz = { gizmoPos.x + qs*0.5f, gizmoPos.y, gizmoPos.z + qs*0.5f };
            XMFLOAT3 yz = { gizmoPos.x, gizmoPos.y + qs*0.5f, gizmoPos.z + qs*0.5f };
            float t;
            if (rayPlane(ray, {0,0,1}, xy, t)) {
                XMVECTOR h = XMVectorAdd(XMLoadFloat3(&ray.origin),
                                         XMVectorScale(XMLoadFloat3(&ray.dir), t));
                XMFLOAT3 hp; XMStoreFloat3(&hp, h);
                if (hp.x >= gizmoPos.x && hp.x <= gizmoPos.x + qs &&
                    hp.y >= gizmoPos.y && hp.y <= gizmoPos.y + qs)
                    return GizmoAxis::XY;
            }
            if (rayPlane(ray, {0,1,0}, xz, t)) {
                XMVECTOR h = XMVectorAdd(XMLoadFloat3(&ray.origin),
                                         XMVectorScale(XMLoadFloat3(&ray.dir), t));
                XMFLOAT3 hp; XMStoreFloat3(&hp, h);
                if (hp.x >= gizmoPos.x && hp.x <= gizmoPos.x + qs &&
                    hp.z >= gizmoPos.z && hp.z <= gizmoPos.z + qs)
                    return GizmoAxis::XZ;
            }
            if (rayPlane(ray, {1,0,0}, yz, t)) {
                XMVECTOR h = XMVectorAdd(XMLoadFloat3(&ray.origin),
                                         XMVectorScale(XMLoadFloat3(&ray.dir), t));
                XMFLOAT3 hp; XMStoreFloat3(&hp, h);
                if (hp.y >= gizmoPos.y && hp.y <= gizmoPos.y + qs &&
                    hp.z >= gizmoPos.z && hp.z <= gizmoPos.z + qs)
                    return GizmoAxis::YZ;
            }
        }
        else if (m_mode == GizmoMode::Rotate) {
            if (rayDisk(ray, {1,0,0}, gizmoPos, cr - ct, cr + ct)) return GizmoAxis::X;
            if (rayDisk(ray, {0,1,0}, gizmoPos, cr - ct, cr + ct)) return GizmoAxis::Y;
            if (rayDisk(ray, {0,0,1}, gizmoPos, cr - ct, cr + ct)) return GizmoAxis::Z;
        }
        else if (m_mode == GizmoMode::Scale) {
            if (rayCylinder(ray, {1,0,0}, gizmoPos, L, r)) return GizmoAxis::X;
            if (rayCylinder(ray, {0,1,0}, gizmoPos, L, r)) return GizmoAxis::Y;
            if (rayCylinder(ray, {0,0,1}, gizmoPos, L, r)) return GizmoAxis::Z;

            float h = 0.12f * scale;
            if (rayCylinder(ray, {0,0,1}, gizmoPos, h, h)) return GizmoAxis::XYZ;
        }

        return GizmoAxis::None;
    }

    XMFLOAT3 GizmoController::dragPlaneIntersect(const Ray& ray) const {
        float t;
        if (rayPlane(ray, m_dragPlaneN, m_dragPlaneP, t)) {
            XMFLOAT3 pt;
            XMStoreFloat3(&pt, XMVectorAdd(
                XMLoadFloat3(&ray.origin),
                XMVectorScale(XMLoadFloat3(&ray.dir), t)));
            return pt;
        }
        return m_dragStart;
    }

    static XMFLOAT3 bestPlaneNormal(GizmoAxis axis,
                                     const XMFLOAT3& camDir)
    {
        switch (axis) {
        case GizmoAxis::X: {
            float dy = fabsf(camDir.y), dz = fabsf(camDir.z);
            return dy > dz ? XMFLOAT3{0,1,0} : XMFLOAT3{0,0,1};
        }
        case GizmoAxis::Y: {
            float dx = fabsf(camDir.x), dz = fabsf(camDir.z);
            return dx > dz ? XMFLOAT3{1,0,0} : XMFLOAT3{0,0,1};
        }
        case GizmoAxis::Z: {
            float dx = fabsf(camDir.x), dy = fabsf(camDir.y);
            return dx > dy ? XMFLOAT3{1,0,0} : XMFLOAT3{0,1,0};
        }
        case GizmoAxis::XY: return {0,0,1};
        case GizmoAxis::XZ: return {0,1,0};
        case GizmoAxis::YZ: return {1,0,0};
        default:            return {0,1,0};
        }
    }

    void GizmoController::applyTranslateDelta(const XMFLOAT3& worldPt,
                                               core::Transform* t) const
    {
        XMVECTOR delta = XMVectorSubtract(XMLoadFloat3(&worldPt),
                                           XMLoadFloat3(&m_dragStart));
        XMVECTOR newPos = XMLoadFloat3(&m_posAtDrag);

        switch (m_dragAxis) {
        case GizmoAxis::X:
            newPos = XMVectorAdd(newPos, XMVectorSet(XMVectorGetX(delta), 0, 0, 0)); break;
        case GizmoAxis::Y:
            newPos = XMVectorAdd(newPos, XMVectorSet(0, XMVectorGetY(delta), 0, 0)); break;
        case GizmoAxis::Z:
            newPos = XMVectorAdd(newPos, XMVectorSet(0, 0, XMVectorGetZ(delta), 0)); break;
        case GizmoAxis::XY:
            newPos = XMVectorAdd(newPos, XMVectorSet(XMVectorGetX(delta), XMVectorGetY(delta), 0, 0)); break;
        case GizmoAxis::XZ:
            newPos = XMVectorAdd(newPos, XMVectorSet(XMVectorGetX(delta), 0, XMVectorGetZ(delta), 0)); break;
        case GizmoAxis::YZ:
            newPos = XMVectorAdd(newPos, XMVectorSet(0, XMVectorGetY(delta), XMVectorGetZ(delta), 0)); break;
        default: return;
        }

        XMFLOAT3 p; XMStoreFloat3(&p, newPos);
        t->setPosition(p);
    }

    void GizmoController::applyRotateDelta(const Ray& ray, core::Transform* t) {
        XMFLOAT3 axisDir = m_dragAxisDir;
        float tt;
        if (!rayPlane(ray, axisDir, m_dragPlaneP, tt)) return;

        XMVECTOR hit   = XMVectorAdd(XMLoadFloat3(&ray.origin),
                                      XMVectorScale(XMLoadFloat3(&ray.dir), tt));
        XMVECTOR toHit = XMVectorSubtract(hit, XMLoadFloat3(&m_dragPlaneP));
        XMVECTOR toDrag= XMVectorSubtract(XMLoadFloat3(&m_dragStart), XMLoadFloat3(&m_dragPlaneP));

        XMVECTOR n    = XMVector3Normalize(XMLoadFloat3(&axisDir));
        toHit  = XMVector3Normalize(XMVectorSubtract(toHit,
                     XMVectorScale(n, XMVectorGetX(XMVector3Dot(toHit, n)))));
        toDrag = XMVector3Normalize(XMVectorSubtract(toDrag,
                     XMVectorScale(n, XMVectorGetX(XMVector3Dot(toDrag, n)))));

        float cosA = XMVectorGetX(XMVector3Dot(toDrag, toHit));
        cosA = std::clamp(cosA, -1.f, 1.f);
        float angle = acosf(cosA);

        XMVECTOR cross = XMVector3Cross(toDrag, toHit);
        if (XMVectorGetX(XMVector3Dot(cross, n)) < 0.f) angle = -angle;

        float totalAngle = m_angleAtDrag + angle;

        XMVECTOR rot = XMQuaternionRotationAxis(n, totalAngle);
        XMVECTOR base = XMLoadFloat4(&XMFLOAT4{0,0,0,1});

        XMFLOAT4 q;
        XMStoreFloat4(&q, XMQuaternionMultiply(rot, base));
        t->setRotationQuat(q);
    }

    void GizmoController::applyScaleDelta(const XMFLOAT3& worldPt,
                                           core::Transform* t) const
    {
        XMVECTOR delta  = XMVectorSubtract(XMLoadFloat3(&worldPt),
                                            XMLoadFloat3(&m_dragStart));
        XMVECTOR axisV  = XMVector3Normalize(XMLoadFloat3(&m_dragAxisDir));
        float    proj   = XMVectorGetX(XMVector3Dot(delta, axisV));
        float    factor = 1.f + proj * 0.5f;

        XMVECTOR baseScale = XMLoadFloat3(&m_scaleAtDrag);
        XMVECTOR newScale  = baseScale;

        switch (m_dragAxis) {
        case GizmoAxis::X:   newScale = XMVectorSetX(baseScale, m_scaleAtDrag.x * factor); break;
        case GizmoAxis::Y:   newScale = XMVectorSetY(baseScale, m_scaleAtDrag.y * factor); break;
        case GizmoAxis::Z:   newScale = XMVectorSetZ(baseScale, m_scaleAtDrag.z * factor); break;
        case GizmoAxis::XYZ: newScale = XMVectorScale(baseScale, factor); break;
        default: return;
        }

        XMFLOAT3 s; XMStoreFloat3(&s, newScale);
        s.x = std::max(s.x, 0.001f);
        s.y = std::max(s.y, 0.001f);
        s.z = std::max(s.z, 0.001f);
        t->setScale(s);
    }

    void GizmoController::onMouseMove(int mx, int my, int vw, int vh,
                                       renderer::Camera* camera)
    {
        if (!m_entity || m_mode == GizmoMode::None) return;

        auto* transform = m_entity->getComponent<core::Transform>();
        if (!transform) return;

        XMFLOAT3 gizmoPos = transform->getPosition();
        XMVECTOR camPos   = XMLoadFloat3(&camera->getPosition());
        float    dist     = XMVectorGetX(XMVector3Length(
                                XMVectorSubtract(camPos, XMLoadFloat3(&gizmoPos))));
        float    scale    = dist * 0.12f;

        Ray ray = buildRay(mx, my, vw, vh, camera);

        if (!m_dragging) {
            m_hoveredAxis = pickAxis(ray, gizmoPos, scale);
            return;
        }

        XMFLOAT3 worldPt = dragPlaneIntersect(ray);

        if (m_mode == GizmoMode::Translate)
            applyTranslateDelta(worldPt, transform);
        else if (m_mode == GizmoMode::Rotate)
            applyRotateDelta(ray, transform);
        else if (m_mode == GizmoMode::Scale)
            applyScaleDelta(worldPt, transform);
    }

    bool GizmoController::onMousePress(int mx, int my, int vw, int vh,
                                        renderer::Camera* camera)
    {
        if (!m_entity || m_mode == GizmoMode::None) return false;

        auto* transform = m_entity->getComponent<core::Transform>();
        if (!transform) return false;

        XMFLOAT3 gizmoPos = transform->getPosition();
        XMVECTOR camPos   = XMLoadFloat3(&camera->getPosition());
        float    dist     = XMVectorGetX(XMVector3Length(
                                XMVectorSubtract(camPos, XMLoadFloat3(&gizmoPos))));
        float    scale    = dist * 0.12f;

        Ray      ray      = buildRay(mx, my, vw, vh, camera);
        GizmoAxis axis    = pickAxis(ray, gizmoPos, scale);

        if (axis == GizmoAxis::None) return false;

        m_dragging     = true;
        m_dragAxis     = axis;
        m_posAtDrag    = transform->getPosition();
        m_scaleAtDrag  = transform->getScale();
        m_dragPlaneP   = gizmoPos;
        m_angleAtDrag  = 0.f;

        XMFLOAT3 camDir;
        XMStoreFloat3(&camDir, XMVector3Normalize(
            XMVectorSubtract(XMLoadFloat3(&gizmoPos), camPos)));

        if (m_mode == GizmoMode::Rotate) {
            switch (axis) {
            case GizmoAxis::X: m_dragAxisDir = {1,0,0}; break;
            case GizmoAxis::Y: m_dragAxisDir = {0,1,0}; break;
            case GizmoAxis::Z: m_dragAxisDir = {0,0,1}; break;
            default:           m_dragAxisDir = {0,1,0}; break;
            }
            m_dragPlaneN = m_dragAxisDir;
        } else if (m_mode == GizmoMode::Scale) {
            switch (axis) {
            case GizmoAxis::X:   m_dragAxisDir = {1,0,0}; break;
            case GizmoAxis::Y:   m_dragAxisDir = {0,1,0}; break;
            case GizmoAxis::Z:   m_dragAxisDir = {0,0,1}; break;
            case GizmoAxis::XYZ: m_dragAxisDir = {1,1,1}; break;
            default:             m_dragAxisDir = {1,0,0}; break;
            }
            m_dragPlaneN = bestPlaneNormal(axis, camDir);
        } else {
            m_dragAxisDir = {};
            m_dragPlaneN  = bestPlaneNormal(axis, camDir);
        }

        float t;
        if (rayPlane(ray, m_dragPlaneN, m_dragPlaneP, t)) {
            XMStoreFloat3(&m_dragStart, XMVectorAdd(
                XMLoadFloat3(&ray.origin),
                XMVectorScale(XMLoadFloat3(&ray.dir), t)));
        } else {
            m_dragStart = gizmoPos;
        }

        return true;
    }

    void GizmoController::onMouseRelease() {
        m_dragging = false;
        m_dragAxis = GizmoAxis::None;
    }

}
