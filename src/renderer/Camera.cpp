#include "Camera.h"

namespace engine::renderer {
    Camera::Camera(const CameraDesc& desc)
    : m_position(desc.position)
    , m_target(desc.target)
    , m_up(desc.up)
    , m_fovY(desc.fovY)
    , m_aspect(16.0f / 9.0f)
    , m_nearZ(desc.nearZ)
    , m_farZ(desc.farZ)
    {
        rebuildView();
        rebuildProjection();
    }

    void Camera::setPosition(const XMFLOAT3& position) {
        m_position = position;
        rebuildView();
    }

    void Camera::setTarget(const XMFLOAT3& target) {
        m_target = target;
        rebuildView();
    }

    void Camera::setAspectRatio(float aspect) {
        m_aspect = aspect;
        rebuildProjection();
    }

    XMMATRIX Camera::getViewMatrix()       const { return m_view; }
    XMMATRIX Camera::getProjectionMatrix() const { return m_projection; }
    const XMFLOAT3& Camera::getPosition()  const { return m_position; }

    void Camera::rebuildView() {
        XMVECTOR eye    = XMLoadFloat3(&m_position);
        XMVECTOR target = XMLoadFloat3(&m_target);
        XMVECTOR up     = XMLoadFloat3(&m_up);

        m_view = XMMatrixLookAtLH(eye, target, up);
    }

    void Camera::rebuildProjection() {
        m_projection = XMMatrixPerspectiveFovLH(
            m_fovY,
            m_aspect,
            m_nearZ,
            m_farZ
        );
    }
}
