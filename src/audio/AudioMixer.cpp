#include "audio/AudioMixer.h"
#include "core/Logger.h"
#include <stdexcept>

namespace engine::audio {

    AudioMixer::AudioMixer(AudioEngine* engine) : m_engine(engine) {
        createChannel("master", 1.f);
        createChannel("sfx",    1.f);
        createChannel("music",  1.f);
        createChannel("ui",     1.f);
    }

    AudioMixer::~AudioMixer() {
        for (auto& [name, data] : m_channels) {
            if (data.voice)
                data.voice->DestroyVoice();
        }
        m_channels.clear();
    }

    void AudioMixer::createChannel(const std::string& name, float volume) {
        if (m_channels.count(name)) return;

        XAUDIO2_VOICE_DETAILS masterDetails{};
        m_engine->getMasterVoice()->GetVoiceDetails(&masterDetails);

        IXAudio2SubmixVoice* submix = nullptr;
        HRESULT hr = m_engine->getXAudio2()->CreateSubmixVoice(
            &submix,
            masterDetails.InputChannels,
            masterDetails.InputSampleRate,
            0, 0, nullptr, nullptr);

        if (FAILED(hr))
            throw std::runtime_error("AudioMixer: failed to create channel: " + name);

        submix->SetVolume(volume);

        m_channels[name] = { submix, volume };
        LOG_INFO("[AudioMixer] channel created: " + name);
    }

    void AudioMixer::destroyChannel(const std::string& name) {
        auto it = m_channels.find(name);
        if (it == m_channels.end()) return;

        if (it->second.voice)
            it->second.voice->DestroyVoice();

        m_channels.erase(it);
    }

    IXAudio2SubmixVoice* AudioMixer::getChannel(const std::string& name) const {
        auto it = m_channels.find(name);
        return (it != m_channels.end()) ? it->second.voice : nullptr;
    }

    void AudioMixer::setChannelVolume(const std::string& name, float volume) {
        auto it = m_channels.find(name);
        if (it == m_channels.end()) return;

        it->second.volume = volume;
        it->second.voice->SetVolume(volume);
    }

    float AudioMixer::getChannelVolume(const std::string& name) const {
        auto it = m_channels.find(name);
        return (it != m_channels.end()) ? it->second.volume : 0.f;
    }

    bool AudioMixer::hasChannel(const std::string& name) const {
        return m_channels.count(name) > 0;
    }

    IXAudio2MasteringVoice* AudioMixer::getMasterVoice() const {
        return m_engine->getMasterVoice();
    }
}