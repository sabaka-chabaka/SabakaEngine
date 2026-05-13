#pragma once
#include "platform/Window.h"
#include "renderer/GraphicsDevice.h"
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

    private:
        std::unique_ptr<platform::Window>           m_window;
        std::unique_ptr<renderer::GraphicsDevice>   m_graphics;
    };
}