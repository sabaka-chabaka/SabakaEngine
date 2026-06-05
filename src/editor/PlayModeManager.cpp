#include "editor/PlayModeManager.h"
#include "editor/EditorApplication.h"
#include "editor/SceneSerializer.h"
#include "core/Scene.h"
#include "core/Logger.h"

namespace engine::editor {

    void PlayModeManager::setApp(EditorApplication* app) {
        m_app = app;
    }

    void PlayModeManager::play() {
        if (!m_app) return;

        if (m_state == PlayState::Stopped) {
            m_snapshot = SceneSerializer::serializeToString(m_app->getScene());
            LOG_INFO("[PlayMode] snapshot saved, entering Play");
        } else if (m_state == PlayState::Paused) {
            LOG_INFO("[PlayMode] resuming from Pause");
        }

        m_state = PlayState::Playing;
        m_app->setSimulating(true);
    }

    void PlayModeManager::pause() {
        if (m_state != PlayState::Playing) return;
        m_state = PlayState::Paused;
        m_app->setSimulating(false);
        LOG_INFO("[PlayMode] paused");
    }

    void PlayModeManager::stop() {
        if (m_state == PlayState::Stopped) return;

        m_app->setSimulating(false);
        m_app->clearScene();

        if (!m_snapshot.empty()) {
            SceneSerializer::deserializeFromString(m_app, m_snapshot);
            m_snapshot.clear();
            LOG_INFO("[PlayMode] scene restored from snapshot");
        }

        m_state = PlayState::Stopped;
    }

}
