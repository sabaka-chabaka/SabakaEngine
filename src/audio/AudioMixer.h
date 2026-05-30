#pragma once
#include "audio/AudioEngine.h"
#include <xaudio2.h>
#include <string>
#include <unordered_map>

namespace engine::audio {

    class AudioMixer {
    public:
        explicit AudioMixer(AudioEngine* engine);
        ~AudioMixer();

        AudioMixer(const AudioMixer&)            = delete;
        AudioMixer& operator=(const AudioMixer&) = delete;

        void createChannel(const std::string& name, float volume = 1.f);
        void destroyChannel(const std::string& name);

        IXAudio2SubmixVoice* getChannel(const std::string& name) const;

        void   setChannelVolume(const std::string& name, float volume);
        float  getChannelVolume(const std::string& name) const;

        bool hasChannel(const std::string& name) const;

        IXAudio2MasteringVoice* getMasterVoice() const;

    private:
        struct ChannelData {
            IXAudio2SubmixVoice* voice  = nullptr;
            float                volume = 1.f;
        };

        AudioEngine*                                   m_engine = nullptr;
        std::unordered_map<std::string, ChannelData>   m_channels;
    };

}