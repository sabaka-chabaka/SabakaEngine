#pragma once
#include <string>

namespace engine::editor {

    class EditorApplication;

    enum class PlayState { Stopped, Playing, Paused };

    class PlayModeManager {
    public:
        PlayModeManager() = default;

        void setApp(EditorApplication* app);

        void play();
        void pause();
        void stop();

        PlayState state() const { return m_state; }
        bool isPlaying() const  { return m_state == PlayState::Playing; }
        bool isPaused()  const  { return m_state == PlayState::Paused;  }
        bool isStopped() const  { return m_state == PlayState::Stopped; }

    private:
        EditorApplication* m_app    = nullptr;
        PlayState          m_state  = PlayState::Stopped;
        std::string        m_snapshot;
    };

}
