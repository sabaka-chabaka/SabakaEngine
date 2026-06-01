#define NOMINMAX
#include "editor/EditorApplication.h"
#include "core/Logger.h"
#include "renderer/GraphicsDevice.h"
#include "platform/Window.h"
#include <stdexcept>

namespace engine::editor {

    EditorApplication::EditorApplication(HWND viewportHwnd, int width, int height) {
        renderer::GraphicsDeviceDesc desc;
        desc.hwnd   = viewportHwnd;
        desc.width  = width;
        desc.height = height;
        desc.vsync  = false;

        m_graphics = std::make_unique<renderer::GraphicsDevice>(desc);
        LOG_INFO("[EditorApplication] GraphicsDevice created on Qt viewport HWND");

        initRenderPipeline(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

        LOG_INFO("[EditorApplication] ready — calling onInit()");
        onInit();
    }

    void EditorApplication::onInit() {
        LOG_INFO("[EditorApplication] scene is empty — load via Asset Browser");
    }

    void EditorApplication::tick(float deltaTime) {
        m_graphics->beginFrame(0.08f, 0.08f, 0.12f);
        renderFrame(deltaTime);
        m_graphics->endFrame();
    }

}