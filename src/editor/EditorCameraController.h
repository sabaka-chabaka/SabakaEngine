#pragma once
#include <DirectXMath.h>

namespace engine::renderer { class Camera; }

namespace engine::editor {

    class EditorCameraController {
    public:
        EditorCameraController() = default;

        void setCamera(renderer::Camera* camera);

        void onMouseMove(int dx, int dy);
        void onScroll(float delta);
        void update(float deltaTime);

        void setMoveSpeed(float speed)   { m_moveSpeed = speed; }
        void setLookSpeed(float speed)   { m_lookSpeed = speed; }
        bool isActive() const            { return m_active; }
        void setActive(bool active);

    private:
        DirectX::XMFLOAT3 forward() const;
        DirectX::XMFLOAT3 right()   const;

        renderer::Camera* m_camera    = nullptr;

        DirectX::XMFLOAT3 m_position  = { 0.f, 1.5f, -15.f };
        float             m_yaw       = 0.f;
        float             m_pitch     = 0.f;

        float             m_moveSpeed = 8.f;
        float             m_lookSpeed = 0.002f;

        bool m_active  = false;
    };

}