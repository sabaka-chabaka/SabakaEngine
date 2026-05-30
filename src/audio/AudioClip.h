#pragma once
#include <xaudio2.h>
#include <cstdint>
#include <string>
#include <vector>

namespace engine::audio {

    class AudioClip {
    public:
        explicit AudioClip(const std::string& path);

        AudioClip(const AudioClip&)            = delete;
        AudioClip& operator=(const AudioClip&) = delete;

        const WAVEFORMATEX&           getFormat() const { return m_format; }
        const std::vector<uint8_t>&   getPcmData() const { return m_pcmData; }
        float                         getDuration() const { return m_durationSeconds; }
        const std::string&            getPath() const { return m_path; }

    private:
        void loadWav(const std::string& path);

        WAVEFORMATEX          m_format         = {};
        std::vector<uint8_t>  m_pcmData;
        float                 m_durationSeconds = 0.f;
        std::string           m_path;
    };

}