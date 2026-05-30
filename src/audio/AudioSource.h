#pragma once
#include "core/Component.h"
#include "audio/AudioClip.h"
#include "audio/AudioEngine.h"
#include <xaudio2.h>
#include <x3daudio.h>
#include <DirectXMath.h>
#include <memory>
#include <string>

namespace engine::audio {

    class AudioMixer;

    class AudioSource : public core::Component {
    public:
        AudioSource(AudioEngine* engine, AudioMixer* mixer,
                    std::shared_ptr<AudioClip> clip,
                    const std::string& mixerChannel = "master");

        ~AudioSource() override;

        void onUpdate(float deltaTime) override;

        void play();
        void pause();
        void stop();
        void restart();

        bool isPlaying() const;

        void setLoop(bool loop);
        void setVolume(float volume);
        void setPitch(float pitch);
        void setSpatial(bool spatial);
        void setMinDistance(float dist);
        void setMaxDistance(float dist);

        bool  getLoop()        const { return m_loop; }
        float getVolume()      const { return m_volume; }
        float getPitch()       const { return m_pitch; }
        bool  isSpatial()      const { return m_spatial; }
        float getMinDistance() const { return m_minDistance; }
        float getMaxDistance() const { return m_maxDistance; }

    private:
        void createVoice();
        void destroyVoice();
        void submitBuffer();
        void updateSpatial();

        AudioEngine*               m_engine       = nullptr;
        AudioMixer*                m_mixer        = nullptr;
        std::shared_ptr<AudioClip> m_clip;
        std::string                m_channel;

        IXAudio2SourceVoice*       m_sourceVoice  = nullptr;

        bool  m_loop        = false;
        float m_volume      = 1.f;
        float m_pitch       = 1.f;
        bool  m_spatial     = false;
        float m_minDistance = 1.f;
        float m_maxDistance = 100.f;

        DirectX::XMFLOAT3 m_prevPosition = { 0.f, 0.f, 0.f };
        float             m_elapsed      = 0.f;
    };

}