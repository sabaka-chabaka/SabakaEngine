#pragma once
#include <xaudio2.h>
#include <x3daudio.h>
#include <wrl/client.h>
#include <cstdint>
#include <string>

namespace engine::audio {

    class AudioEngine {
    public:
        AudioEngine();
        ~AudioEngine();

        AudioEngine(const AudioEngine&)            = delete;
        AudioEngine& operator=(const AudioEngine&) = delete;

        void update(float deltaTime);

        IXAudio2*               getXAudio2()      const { return m_xaudio2.Get(); }
        IXAudio2MasteringVoice* getMasterVoice()  const { return m_masterVoice; }
        const X3DAUDIO_HANDLE&  getX3DHandle()    const { return m_x3dHandle; }

        uint32_t getOutputChannels() const { return m_outputChannels; }

        void setMasterVolume(float volume);
        float getMasterVolume() const;

    private:
        Microsoft::WRL::ComPtr<IXAudio2> m_xaudio2;
        IXAudio2MasteringVoice*          m_masterVoice    = nullptr;
        X3DAUDIO_HANDLE                  m_x3dHandle      = {};
        uint32_t                         m_outputChannels = 2;
        float                            m_masterVolume   = 1.f;
    };
}