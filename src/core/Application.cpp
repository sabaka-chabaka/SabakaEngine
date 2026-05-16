#include "core/Application.h"
#include "core/Logger.h"
#include <chrono>

#include "renderer/Mesh.h"
#include "renderer/Shader.h"

namespace engine::renderer {
    struct InputElementDesc;
    struct Vertex;
}

namespace engine::core {

    Application::Application() {

        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

        Logger::get().openLogFile("engine.log");
        Logger::get().setMinLevel(LogLevel::Trace);

        LOG_INFO("Initializing SabakaEngine");

        platform::WindowDesc wDesc;
        wDesc.title  = L"SabakaEngine v0.1";
        wDesc.width  = 1280;
        wDesc.height = 720;

        m_window = std::make_unique<platform::Window>(wDesc);
        LOG_INFO("Window created (1280x720)");

        renderer::GraphicsDeviceDesc gDesc;
        gDesc.hwnd   = m_window->getNativeHandle();
        gDesc.width  = m_window->getWidth();
        gDesc.height = m_window->getHeight();
        gDesc.vsync  = true;

        m_graphics = std::make_unique<renderer::GraphicsDevice>(gDesc);
        LOG_INFO("DirectX 11 device initialized");

        m_window->setResizeCallback([this](int w, int h) {
            LOG_DEBUG("Window resized: " + std::to_string(w) + "x" + std::to_string(h));
            m_graphics->onResize(w, h);
        });

        std::vector<renderer::Vertex> vertices = {
            {  0.0f,  0.5f, 0.0f,   1.0f, 0.2f, 0.2f },
            {  0.5f, -0.5f, 0.0f,   0.2f, 1.0f, 0.2f },
            { -0.5f, -0.5f, 0.0f,   0.2f, 0.2f, 1.0f },
        };

        std::vector<unsigned int> indices = { 0, 1, 2 };

        m_mesh = std::make_unique<renderer::Mesh>(
           m_graphics->getDevice(),
           m_graphics->getDeviceContext(),
           vertices,
           indices
        );

        std::vector<renderer::InputElementDesc> layout = {
           { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0  },
           { "COLOR",    DXGI_FORMAT_R32G32B32_FLOAT, 12 },
        };

        m_shader = std::make_unique<renderer::Shader>(
            m_graphics->getDevice(),
            L"shaders/Basic.vs.hlsl",
            L"shaders/Basic.ps.hlsl",
            layout
        );

        LOG_INFO("Scene resources ready");
    }

    int Application::run() {
        using Clock     = std::chrono::high_resolution_clock;
        using Duration  = std::chrono::duration<float>;

        LOG_INFO("Entering main loop");

        auto lastTime = Clock::now();

        while (m_window->processMessages()) {
            auto  now       = Clock::now();
            float deltaTime = Duration(now - lastTime).count();
            lastTime        = now;

            onUpdate(deltaTime);

            m_graphics->beginFrame(0.1f, 0.1f, 0.15f);

            m_shader->bind(m_graphics->getDeviceContext());
            m_mesh->draw();

            onRender();
            m_graphics->endFrame();
        }

        LOG_INFO("Main loop exited");
        return 0;
    }

}