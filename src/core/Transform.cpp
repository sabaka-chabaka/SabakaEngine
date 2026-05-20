#include "Transform.h"

namespace engine::core {
    Transform::Transform()
       : m_position(0.0f, 0.0f, 0.0f)
       , m_rotation(0.0f, 0.0f, 0.0f, 1.0f)
       , m_scale(1.0f, 1.0f, 1.0f)
       , m_worldMatrix(XMMatrixIdentity())
       , m_normalMatrix(XMMatrixIdentity())
       , m_dirty(true)
    {}

    void Transform::setPosition(float x, float y, float z) {
        m_position = { x, y, z };
        markDirty();
    }

    void Transform::setPosition(const XMFLOAT3 &position) {
        m_position = position;
        markDirty();
    }

    void Transform::setRotationEuler(float pitch, float yaw, float roll) {
        XMVECTOR q = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
        XMStoreFloat4(&m_rotation, q);
        markDirty();
    }

    void Transform::setRotationEuler(const XMFLOAT3& eulerRadians) {
        setRotationEuler(eulerRadians.x, eulerRadians.y, eulerRadians.z);
    }

    void Transform::setScale(float x, float y, float z) {
        m_scale = { x, y, z };
        markDirty();
    }

    void Transform::setScale(const XMFLOAT3& scale) {
        m_scale = scale;
        markDirty();
    }

    void Transform::setScale(float uniform) {
        m_scale = { uniform, uniform, uniform };
        markDirty();
    }

    void Transform::translate(float x, float y, float z) {
        m_position.x += x;
        m_position.y += y;
        m_position.z += z;
        markDirty();
    }

    void Transform::translate(const XMFLOAT3& delta) {
        translate(delta.x, delta.y, delta.z);
    }

    void Transform::rotateEuler(float pitch, float yaw, float roll) {
        XMVECTOR current = XMLoadFloat4(&m_rotation);
        XMVECTOR delta   = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
        XMVECTOR result  = XMQuaternionMultiply(current, delta);
        result           = XMQuaternionNormalize(result);
        XMStoreFloat4(&m_rotation, result);
        markDirty();
    }

    void Transform::rotateEuler(const XMFLOAT3& deltaEulerRadians) {
        rotateEuler(deltaEulerRadians.x, deltaEulerRadians.y, deltaEulerRadians.z);
    }

    XMFLOAT3 Transform::getPosition() const {
        return m_position;
    }

    XMFLOAT3 Transform::getRotationEuler() const {
        XMVECTOR q = XMLoadFloat4(&m_rotation);
        XMMATRIX R = XMMatrixRotationQuaternion(q);

        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, R);

        float pitch = asinf(-m._32);
        float yaw, roll;

        if (fabsf(m._32) < 0.9999f) {
            yaw  = atan2f(m._31, m._33);
            roll = atan2f(m._12, m._22);
        } else {
            yaw  = atan2f(-m._13, m._11);
            roll = 0.0f;
        }

        return { pitch, yaw, roll };
    }

    XMFLOAT3 Transform::getScale() const {
        return m_scale;
    }

    XMMATRIX Transform::getWorldMatrix() const {
        rebuildIfDirty();
        return m_worldMatrix;
    }

    XMMATRIX Transform::getNormalMatrix() const {
        rebuildIfDirty();
        return m_normalMatrix;
    }

    void Transform::markDirty() {
        m_dirty = true;
    }

    void Transform::rebuildIfDirty() const {
        if (!m_dirty) return;

        XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
        XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&m_rotation));
        XMMATRIX T = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

        m_worldMatrix  = S * R * T;
        m_normalMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, m_worldMatrix));

        m_dirty = false;
    }
}
