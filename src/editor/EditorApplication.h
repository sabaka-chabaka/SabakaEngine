#pragma once
#include "core/Application.h"
#include <string>
#include <windows.h>

namespace engine::core { class Entity; }

namespace engine::editor {

    class EditorApplication : public core::Application {
    public:
        explicit EditorApplication(HWND viewportHwnd, int width, int height);
        ~EditorApplication() = default;

        void tick(float deltaTime);

        renderer::GraphicsDevice* getDevice() const { return m_graphics.get(); }
        core::Scene*              getScene()  const { return m_scene.get(); }

        core::Entity* createCube(const std::string& name = "Cube");
        core::Entity* createEmpty(const std::string& name = "Entity");
        void          destroyEntity(core::Entity* entity);
        void          selectEntity(core::Entity* entity) { m_selectedEntity = entity; }
        core::Entity* getSelectedEntity() const { return m_selectedEntity; }

    protected:
        void onInit() override;

    private:
        core::Entity* m_selectedEntity = nullptr;
    };

}