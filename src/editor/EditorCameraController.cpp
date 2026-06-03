#define NOMINMAX
#include "editor/EditorCameraController.h"
#include "renderer/Camera.h"
#include <Qt>
#include <algorithm>
#include <cmath>

using namespace DirectX;

namespace engine::editor {

    static constexpr float kPitchLimit = XM_PIDIV2 - 0.01f;
    static constexpr float kDegToRad   = XM_PI / 180.f;

    void EditorCameraController::setCamera(renderer::Camera* camera) {
        m_camera = camera;
        if (!m_camera) return;
        XMFLOAT3 pos = m_camera->getPosition();
        m_position   = pos;
        m_yaw        = 0.f;
        m_pitch      = 0.f;
    }

    void EditorCameraController::setActive(bool active) {
        m_active = active;
        if (!active) {
            m_keyW = m_keyA = m_keyS = m_keyD = m_keyQ = m_keyE = m_keyShift = false;
        }
    }

    void EditorCameraController::onMouseMove(int dx, int dy) {
        if (!m_active || !m_camera) return;
        m_yaw   += static_cast<float>(dx) * m_lookSpeed * kDegToRad;
        m_pitch += static_cast<float>(dy) * m_lookSpeed * kDegToRad;
        m_pitch  = std::clamp(m_pitch, -kPitchLimit, kPitchLimit);
    }

    void EditorCameraController::setKey(int qtKey, bool pressed) {
        switch (qtKey) {
            case Qt::Key_W: m_keyW     = pressed; break;
            case Qt::Key_A: m_keyA     = pressed; break;
            case Qt::Key_S: m_keyS     = pressed; break;
            case Qt::Key_D: m_keyD     = pressed; break;
            case Qt::Key_Q: m_keyQ     = pressed; break;
            case Qt::Key_E: m_keyE     = pressed; break;
            case Qt::Key_Shift:
            case Qt::Key_Control: m_keyShift = pressed; break;
            default: break;
        }
    }

    void EditorCameraController::onScroll(float delta) {
        if (!m_camera) return;
        XMFLOAT3 fwd = forward();
        float    step = delta * m_moveSpeed * 0.3f;
        XMVECTOR pos  = XMLoadFloat3(&m_position);
        pos = XMVectorAdd(pos, XMVectorScale(XMLoadFloat3(&fwd), step));
        XMStoreFloat3(&m_position, pos);

        XMFLOAT3 target = {
            m_position.x + fwd.x,
            m_position.y + fwd.y,
            m_position.z + fwd.z
        };
        m_camera->setPosition(m_position);
        m_camera->setTarget(target);
    }

    XMFLOAT3 EditorCameraController::forward() const {
        float cy = cosf(m_yaw),   sy = sinf(m_yaw);
        float cp = cosf(m_pitch), sp = sinf(m_pitch);
        return { sy * cp, -sp, cy * cp };
    }

    XMFLOAT3 EditorCameraController::right() const {
        XMFLOAT3 fwd = forward();
        XMVECTOR f   = XMLoadFloat3(&fwd);
        XMVECTOR up  = XMVectorSet(0.f, 1.f, 0.f, 0.f);
        XMVECTOR r   = XMVector3Normalize(XMVector3Cross(f, up));
        XMFLOAT3 out;
        XMStoreFloat3(&out, r);
        return out;
    }

    void EditorCameraController::update(float deltaTime) {
        if (!m_active || !m_camera) return;

        float speed = m_moveSpeed * (m_keyShift ? 3.f : 1.f) * deltaTime;

        XMFLOAT3 fwd = forward();
        XMFLOAT3 rgt = right();

        XMVECTOR pos = XMLoadFloat3(&m_position);

        if (m_keyW) pos = XMVectorAdd(pos, XMVectorScale(XMLoadFloat3(&fwd),  speed));
        if (m_keyS) pos = XMVectorAdd(pos, XMVectorScale(XMLoadFloat3(&fwd), -speed));
        if (m_keyD) pos = XMVectorAdd(pos, XMVectorScale(XMLoadFloat3(&rgt),  speed));
        if (m_keyA) pos = XMVectorAdd(pos, XMVectorScale(XMLoadFloat3(&rgt), -speed));
        if (m_keyE) pos = XMVectorAdd(pos, XMVectorSet(0.f,  speed, 0.f, 0.f));
        if (m_keyQ) pos = XMVectorAdd(pos, XMVectorSet(0.f, -speed, 0.f, 0.f));

        XMStoreFloat3(&m_position, pos);

        XMFLOAT3 target = {
            m_position.x + fwd.x,
            m_position.y + fwd.y,
            m_position.z + fwd.z
        };

        m_camera->setPosition(m_position);
        m_camera->setTarget(target);
    }

}