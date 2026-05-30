#pragma once
#include "core/Application.h"
#include <windows.h>

namespace engine::editor {

    class EditorApplication : public core::Application {
    public:
        explicit EditorApplication(HWND viewportHwnd, int width, int height);
        ~EditorApplication() = default;

        void tick(float deltaTime);

        renderer::GraphicsDevice* getDevice() const { return m_graphics.get(); }

    protected:
        void onInit() override;
    };

}