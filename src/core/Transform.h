#pragma once
#include <DirectXMath.h>

#include "Component.h"

namespace engine::core {
    using namespace DirectX;

    class Transform : public Component {
    public:
        Transform();

        void setPosition(float x, float y, float z);
        void setPosition(const XMFLOAT3& position);

        void setRotationEuler(float pitch, float yaw, float roll);
        void setRotationEuler(const XMFLOAT3& eulerRadians);

        void setScale(float x, float y, float z);
        void setScale(const XMFLOAT3& scale);
        void setScale(float uniform);

        void translate(float x, float y, float z);
        void translate(const XMFLOAT3& delta);

        void rotateEuler(float pitch, float yaw, float roll);
        void rotateEuler(const XMFLOAT3& deltaEulerRadians);

        XMFLOAT3 getPosition()      const;
        XMFLOAT3 getRotationEuler() const;
        XMFLOAT3 getScale()         const;

        XMMATRIX getWorldMatrix()  const;
        XMMATRIX getNormalMatrix() const;

    private:
        void markDirty();
        void rebuildIfDirty() const;

        XMFLOAT3 m_position;
        XMFLOAT4 m_rotation;
        XMFLOAT3 m_scale;

        mutable XMMATRIX m_worldMatrix;
        mutable XMMATRIX m_normalMatrix;
        mutable bool     m_dirty;
    };
}
