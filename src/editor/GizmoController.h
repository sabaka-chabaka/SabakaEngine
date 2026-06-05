#pragma once
#define NOMINMAX
#include "editor/GizmoMode.h"
#include <DirectXMath.h>

namespace engine::core    { class Entity; class Transform; }
namespace engine::renderer { class Camera; }

namespace engine::editor {

    class GizmoController {
    public:
        GizmoController() = default;

        void setMode(GizmoMode mode)           { m_mode = mode; }
        GizmoMode  getMode()  const            { return m_mode; }
        GizmoAxis  getHovered() const          { return m_hoveredAxis; }
        bool       isDragging() const          { return m_dragging; }

        void setEntity(core::Entity* entity);

        void onMouseMove(int mouseX, int mouseY,
                         int viewW,  int viewH,
                         renderer::Camera* camera);

        bool onMousePress(int mouseX, int mouseY,
                          int viewW,  int viewH,
                          renderer::Camera* camera);

        void onMouseRelease();

    private:
        struct Ray {
            DirectX::XMFLOAT3 origin;
            DirectX::XMFLOAT3 dir;
        };

        Ray buildRay(int mouseX, int mouseY,
                     int viewW,  int viewH,
                     renderer::Camera* camera) const;

        GizmoAxis pickAxis(const Ray& ray,
                           const DirectX::XMFLOAT3& gizmoPos,
                           float scale) const;

        bool rayPlane(const Ray& ray,
                      const DirectX::XMFLOAT3& planeN,
                      const DirectX::XMFLOAT3& planeP,
                      float& t) const;

        bool rayCylinder(const Ray& ray,
                         const DirectX::XMFLOAT3& axisDir,
                         const DirectX::XMFLOAT3& origin,
                         float length, float radius) const;

        bool rayDisk(const Ray& ray,
                     const DirectX::XMFLOAT3& normal,
                     const DirectX::XMFLOAT3& center,
                     float innerR, float outerR) const;

        DirectX::XMFLOAT3 dragPlaneIntersect(const Ray& ray) const;

        void applyTranslateDelta(const DirectX::XMFLOAT3& worldPt,
                                 core::Transform* t) const;
        void applyRotateDelta(const Ray& ray,
                              core::Transform* t);
        void applyScaleDelta(const DirectX::XMFLOAT3& worldPt,
                             core::Transform* t) const;

        core::Entity*     m_entity     = nullptr;
        GizmoMode         m_mode       = GizmoMode::None;
        GizmoAxis         m_hoveredAxis = GizmoAxis::None;
        GizmoAxis         m_dragAxis   = GizmoAxis::None;
        bool              m_dragging   = false;

        DirectX::XMFLOAT3 m_dragPlaneN  = {};
        DirectX::XMFLOAT3 m_dragPlaneP  = {};
        DirectX::XMFLOAT3 m_dragStart   = {};
        DirectX::XMFLOAT3 m_posAtDrag   = {};
        DirectX::XMFLOAT3 m_scaleAtDrag = {};
        float             m_angleAtDrag  = 0.f;
        DirectX::XMFLOAT3 m_dragAxisDir  = {};
    };

}
