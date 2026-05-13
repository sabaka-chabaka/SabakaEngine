#include "core/Application.h"
#include <chrono>

namespace engine::core {

    Application::Application() {
        platform::WindowDesc wDesc;
        wDesc.title  = L"GameEngine v0.1";
        wDesc.width  = 1280;
        wDesc.height = 720;

        m_window = std::make_unique<platform::Window>(wDesc);

        renderer::GraphicsDeviceDesc gDesc;
        gDesc.hwnd   = m_window->getNativeHandle();
        gDesc.width  = m_window->getWidth();
        gDesc.height = m_window->getHeight();
        gDesc.vsync  = true;

        m_graphics = std::make_unique<renderer::GraphicsDevice>(gDesc);

        m_window->setResizeCallback([this](int w, int h) {
            m_graphics->onResize(w, h);
        });
    }

    int Application::run() {
        using Clock     = std::chrono::high_resolution_clock;
        using Duration  = std::chrono::duration<float>;

        auto lastTime = Clock::now();

        while (m_window->processMessages()) {
            auto  now       = Clock::now();
            float deltaTime = Duration(now - lastTime).count();
            lastTime        = now;

            onUpdate(deltaTime);

            m_graphics->beginFrame(0.1f, 0.1f, 0.15f);
            onRender();
            m_graphics->endFrame();
        }

        return 0;
    }

}