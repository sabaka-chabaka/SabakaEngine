#define NOMINMAX
#include "editor/EditorApplication.h"
#include "core/Logger.h"
#include "core/Entity.h"
#include "core/Transform.h"
#include "core/MeshRenderer.h"
#include "core/BoundingBoxComponent.h"
#include "math/AABB.h"
#include "renderer/GraphicsDevice.h"
#include <stdexcept>

namespace engine::editor {

    EditorApplication::EditorApplication(HWND viewportHwnd, int width, int height)
        : Application(EditorTag{})
    {
        Logger::get().openLogFile("editor.log");
        Logger::get().setMinLevel(LogLevel::Debug);

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

    core::Entity* EditorApplication::createCube(const std::string& name) {
        core::Entity* e = m_scene->createEntity(name);
        auto* t = e->addComponent<core::Transform>();
        t->setPosition(0.f, 0.f, 0.f);
        t->setScale(1.f, 1.f, 1.f);

        auto* bb = e->addComponent<core::BoundingBoxComponent>();
        math::AABB aabb;
        aabb.min = { -0.5f, -0.5f, -0.5f };
        aabb.max = {  0.5f,  0.5f,  0.5f };
        bb->setLocalAABB(aabb);

        m_selectedEntity = e;
        LOG_INFO("[EditorApplication] created cube: " + name);
        return e;
    }

    core::Entity* EditorApplication::createEmpty(const std::string& name) {
        core::Entity* e = m_scene->createEntity(name);
        e->addComponent<core::Transform>();
        m_selectedEntity = e;
        LOG_INFO("[EditorApplication] created entity: " + name);
        return e;
    }

    void EditorApplication::destroyEntity(core::Entity* entity) {
        if (!entity) return;
        if (m_selectedEntity == entity) m_selectedEntity = nullptr;
        m_scene->destroyEntity(entity);
    }

    void EditorApplication::tick(float deltaTime) {
        m_graphics->beginFrame(0.08f, 0.08f, 0.12f);
        renderFrame(deltaTime);
        m_graphics->endFrame();
    }

}