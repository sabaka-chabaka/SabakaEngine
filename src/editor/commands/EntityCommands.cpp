#include "editor/commands/EntityCommands.h"
#include "editor/EditorApplication.h"
#include "editor/SceneSerializer.h"
#include "core/Scene.h"
#include "core/Entity.h"
#include "core/Transform.h"
#include "core/Logger.h"
#include <json.hpp>

using json = nlohmann::json;

namespace engine::editor {

    CreateEntityCommand::CreateEntityCommand(EditorApplication* app,
                                             std::string name,
                                             bool asCube)
        : m_app(app)
        , m_name(std::move(name))
        , m_asCube(asCube)
    {}

    void CreateEntityCommand::execute() {
        core::Entity* e = m_asCube
            ? m_app->createCube(m_name)
            : m_app->createEmpty(m_name);
        m_createdId = e->getId();
    }

    void CreateEntityCommand::undo() {
        if (m_createdId == 0) return;
        core::Entity* e = m_app->getScene()->findById(m_createdId);
        if (e) m_app->destroyEntity(e);
        m_createdId = 0;
    }

    std::string CreateEntityCommand::description() const {
        return "Create " + m_name;
    }

    DestroyEntityCommand::DestroyEntityCommand(EditorApplication* app,
                                               uint64_t entityId,
                                               std::string snapshot)
        : m_app(app)
        , m_entityId(entityId)
        , m_snapshot(std::move(snapshot))
    {
        core::Entity* e = app->getScene()->findById(entityId);
        m_name = e ? e->getName() : "Entity";
    }

    void DestroyEntityCommand::execute() {
        core::Entity* e = m_app->getScene()->findById(m_entityId);
        if (e) m_app->destroyEntity(e);
    }

    void DestroyEntityCommand::undo() {
        SceneSerializer::deserializeFromString(m_app, m_snapshot);
    }

    std::string DestroyEntityCommand::description() const {
        return "Destroy " + m_name;
    }

}
