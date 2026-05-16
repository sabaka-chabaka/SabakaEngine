#pragma once
#include "platform/Window.h"
#include "renderer/GraphicsDevice.h"
#include "renderer/Mesh.h"
#include "renderer/Shader.h"
#include "renderer/Camera.h"
#include "renderer/ConstantBuffer.h"
#include "renderer/TransformData.h"
#include <memory>


namespace engine::core {
    class Application {
    public:
        Application();
        ~Application() = default;

        Application(const Application&)            = delete;
        Application& operator=(const Application&) = delete;

        int run();

    protected:
        virtual void onUpdate(float deltaTime) {}
        virtual void onRender()               {}

        std::unique_ptr<platform::Window>                                  m_window;
        std::unique_ptr<renderer::GraphicsDevice>                          m_graphics;
        std::unique_ptr<renderer::Mesh>                                    m_mesh;
        std::unique_ptr<renderer::Shader>                                  m_shader;
        std::unique_ptr<renderer::Camera>                                  m_camera;
        std::unique_ptr<renderer::ConstantBuffer<renderer::TransformData>> m_transformCB;
    };
}