#pragma once
#include <DirectXMath.h>

namespace engine::renderer {
    using namespace DirectX;

    struct CameraDesc {
        XMFLOAT3 position  = { 0.0f, 1.0f, -5.0f };
        XMFLOAT3 target    = { 0.0f, 0.0f,  0.0f };
        XMFLOAT3 up        = { 0.0f, 1.0f,  0.0f };
        float    fovY      = XM_PIDIV4;
        float    nearZ     = 0.1f;
        float    farZ      = 1000.0f;
    };

    class Camera {
    public:
        explicit Camera(const CameraDesc& desc);

        void setPosition(const XMFLOAT3& position);
        void setTarget(const XMFLOAT3& target);
        void setAspectRatio(float aspect);

        XMMATRIX getViewMatrix()       const;
        XMMATRIX getProjectionMatrix() const;

        const XMFLOAT3& getPosition() const;

    private:
        void rebuildView();
        void rebuildProjection();

        XMFLOAT3 m_position;
        XMFLOAT3 m_target;
        XMFLOAT3 m_up;

        float    m_fovY;
        float    m_aspect;
        float    m_nearZ;
        float    m_farZ;

        XMMATRIX m_view;
        XMMATRIX m_projection;
    };
}