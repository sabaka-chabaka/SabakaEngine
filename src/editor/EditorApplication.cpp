#define NOMINMAX
#include "editor/EditorApplication.h"
#include "core/Logger.h"
#include "renderer/GraphicsDevice.h"
#include <stdexcept>

namespace engine::editor {

    EditorApplication::EditorApplication(HWND viewportHwnd, int width, int height)
        : Application(EditorTag{})
    {
        core::Logger::get().openLogFile("editor.log");
        core::Logger::get().setMinLevel(core::LogLevel::Debug);

        renderer::GraphicsDeviceDesc desc;
        desc.hwnd   = viewportHwnd;
        desc.width  = static_cast<uint32_t>(width);
        desc.height = static_cast<uint32_t>(height);
        desc.vsync  = false;

        m_graphics = std::make_unique<renderer::GraphicsDevice>(desc);
        LOG_INFO("[EditorApplication] GraphicsDevice on Qt HWND");

        initCoreSystems();
        initRenderPipeline(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

        onInit();
    }

    void EditorApplication::onInit() {
        LOG_INFO("[EditorApplication] ready");
    }

    void EditorApplication::tick(float deltaTime) {
        m_graphics->beginFrame(0.08f, 0.08f, 0.12f);
        renderFrame(deltaTime);
        m_graphics->endFrame();
    }

}