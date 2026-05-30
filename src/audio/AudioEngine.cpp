#include "audio/AudioEngine.h"
#include "core/Logger.h"
#include <stdexcept>

namespace engine::audio {

    AudioEngine::AudioEngine() {
        HRESULT hr = XAudio2Create(m_xaudio2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
        if (FAILED(hr))
            throw std::runtime_error("AudioEngine: XAudio2Create failed");

        hr = m_xaudio2->CreateMasteringVoice(&m_masterVoice);
        if (FAILED(hr))
            throw std::runtime_error("AudioEngine: CreateMasteringVoice failed");

        XAUDIO2_VOICE_DETAILS details{};
        m_masterVoice->GetVoiceDetails(&details);
        m_outputChannels = details.InputChannels;

        DWORD channelMask = 0;
        m_masterVoice->GetChannelMask(&channelMask);

        X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, m_x3dHandle);

        LOG_INFO("[AudioEngine] initialized — channels: " + std::to_string(m_outputChannels));
    }

    AudioEngine::~AudioEngine() {
        if (m_masterVoice) {
            m_masterVoice->DestroyVoice();
            m_masterVoice = nullptr;
        }
    }

    void AudioEngine::update(float /*deltaTime*/) {}

    void AudioEngine::setMasterVolume(float volume) {
        m_masterVolume = volume;
        if (m_masterVoice)
            m_masterVoice->SetVolume(volume);
    }

    float AudioEngine::getMasterVolume() const {
        return m_masterVolume;
    }
}