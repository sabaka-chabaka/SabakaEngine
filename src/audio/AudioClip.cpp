#include "audio/AudioClip.h"
#include "core/Logger.h"
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace engine::audio {

    namespace {
        uint32_t readU32(std::ifstream& f) {
            uint32_t v = 0;
            f.read(reinterpret_cast<char*>(&v), 4);
            return v;
        }

        uint16_t readU16(std::ifstream& f) {
            uint16_t v = 0;
            f.read(reinterpret_cast<char*>(&v), 2);
            return v;
        }

        uint32_t readTag(std::ifstream& f) {
            return readU32(f);
        }

        constexpr uint32_t makeTag(char a, char b, char c, char d) {
            return static_cast<uint32_t>(a)
                 | static_cast<uint32_t>(b) << 8
                 | static_cast<uint32_t>(c) << 16
                 | static_cast<uint32_t>(d) << 24;
        }
    }

    AudioClip::AudioClip(const std::string& path) : m_path(path) {
        loadWav(path);
    }

    void AudioClip::loadWav(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f)
            throw std::runtime_error("AudioClip: cannot open: " + path);

        const uint32_t TAG_RIFF = makeTag('R','I','F','F');
        const uint32_t TAG_WAVE = makeTag('W','A','V','E');
        const uint32_t TAG_fmt  = makeTag('f','m','t',' ');
        const uint32_t TAG_data = makeTag('d','a','t','a');

        if (readTag(f) != TAG_RIFF)
            throw std::runtime_error("AudioClip: not a RIFF file: " + path);

        readU32(f);

        if (readTag(f) != TAG_WAVE)
            throw std::runtime_error("AudioClip: not a WAVE file: " + path);

        bool gotFmt  = false;
        bool gotData = false;

        while (f && !gotData) {
            uint32_t chunkId   = readTag(f);
            uint32_t chunkSize = readU32(f);

            if (chunkId == TAG_fmt) {
                if (chunkSize < 16)
                    throw std::runtime_error("AudioClip: fmt chunk too small: " + path);

                uint16_t audioFormat   = readU16(f);
                uint16_t numChannels   = readU16(f);
                uint32_t sampleRate    = readU32(f);
                uint32_t byteRate      = readU32(f);
                uint16_t blockAlign    = readU16(f);
                uint16_t bitsPerSample = readU16(f);

                if (audioFormat != WAVE_FORMAT_PCM)
                    throw std::runtime_error("AudioClip: only PCM WAV supported: " + path);

                m_format.wFormatTag      = WAVE_FORMAT_PCM;
                m_format.nChannels       = numChannels;
                m_format.nSamplesPerSec  = sampleRate;
                m_format.nAvgBytesPerSec = byteRate;
                m_format.nBlockAlign     = blockAlign;
                m_format.wBitsPerSample  = bitsPerSample;
                m_format.cbSize          = 0;

                if (chunkSize > 16) {
                    f.seekg(chunkSize - 16, std::ios::cur);
                }

                gotFmt = true;

            } else if (chunkId == TAG_data) {
                if (!gotFmt)
                    throw std::runtime_error("AudioClip: data before fmt chunk: " + path);

                m_pcmData.resize(chunkSize);
                f.read(reinterpret_cast<char*>(m_pcmData.data()), chunkSize);

                uint32_t bytesPerSample = m_format.wBitsPerSample / 8;
                uint32_t totalSamples   = chunkSize / (bytesPerSample * m_format.nChannels);
                m_durationSeconds       = static_cast<float>(totalSamples) / static_cast<float>(m_format.nSamplesPerSec);

                gotData = true;

            } else {
                f.seekg(chunkSize + (chunkSize & 1), std::ios::cur);
            }
        }

        if (!gotFmt || !gotData)
            throw std::runtime_error("AudioClip: incomplete WAV file: " + path);

        LOG_INFO("[AudioClip] loaded: " + path +
            " | " + std::to_string(m_format.nSamplesPerSec) + "Hz"
            " | " + std::to_string(m_format.nChannels) + "ch"
            " | " + std::to_string(m_format.wBitsPerSample) + "bit"
            " | " + std::to_string(m_durationSeconds) + "s");
    }

}