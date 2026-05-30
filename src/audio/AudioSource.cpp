#define NOMINMAX
#include "audio/AudioSource.h"
#include "audio/AudioMixer.h"
#include "core/Entity.h"
#include "core/Transform.h"
#include "core/Logger.h"
#include <cstring>
#include <stdexcept>
#include <algorithm>

using namespace DirectX;

namespace engine::audio {

    static XMFLOAT3 getEntityPos(core::Entity* e) {
        if (auto* t = e->getComponent<core::Transform>())
            return t->getPosition();
        return { 0.f, 0.f, 0.f };
    }

    static XMFLOAT3 getEntityForward(core::Entity* e) {
        if (auto* t = e->getComponent<core::Transform>()) {
            XMFLOAT4 quat = t->getRotationQuat();
            XMMATRIX rot  = XMMatrixRotationQuaternion(XMLoadFloat4(&quat));
            XMVECTOR fwd = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), rot);
            XMFLOAT3 out;
            XMStoreFloat3(&out, fwd);
            return out;
        }
        return { 0.f, 0.f, 1.f };
    }

    AudioSource::AudioSource(AudioEngine* engine, AudioMixer* mixer,
                             std::shared_ptr<AudioClip> clip,
                             const std::string& mixerChannel)
        : m_engine(engine)
        , m_mixer(mixer)
        , m_clip(std::move(clip))
        , m_channel(mixerChannel) {}

    AudioSource::~AudioSource() {
        destroyVoice();
    }

    void AudioSource::createVoice() {
        if (m_sourceVoice) return;
        if (!m_clip)        return;

        XAUDIO2_SEND_DESCRIPTOR sendDesc{};
        sendDesc.Flags = 0;

        IXAudio2SubmixVoice* submix = m_mixer->getChannel(m_channel);
        IXAudio2Voice* outputVoice  = submix
            ? static_cast<IXAudio2Voice*>(submix)
            : static_cast<IXAudio2Voice*>(m_engine->getMasterVoice());
        sendDesc.pOutputVoice = outputVoice;

        XAUDIO2_VOICE_SENDS sends{};
        sends.SendCount = 1;
        sends.pSends    = &sendDesc;

        HRESULT hr = m_engine->getXAudio2()->CreateSourceVoice(
            &m_sourceVoice,
            &m_clip->getFormat(),
            0,
            XAUDIO2_MAX_FREQ_RATIO,
            nullptr,
            &sends,
            nullptr);

        if (FAILED(hr))
            throw std::runtime_error("AudioSource: CreateSourceVoice failed");

        m_sourceVoice->SetVolume(m_volume);
        m_sourceVoice->SetFrequencyRatio(m_pitch);
    }

    void AudioSource::destroyVoice() {
        if (!m_sourceVoice) return;
        m_sourceVoice->Stop();
        m_sourceVoice->FlushSourceBuffers();
        m_sourceVoice->DestroyVoice();
        m_sourceVoice = nullptr;
    }

    void AudioSource::submitBuffer() {
        if (!m_sourceVoice || !m_clip) return;

        XAUDIO2_BUFFER buf{};
        buf.Flags      = XAUDIO2_END_OF_STREAM;
        buf.AudioBytes = static_cast<UINT32>(m_clip->getPcmData().size());
        buf.pAudioData = m_clip->getPcmData().data();
        buf.LoopCount  = m_loop ? XAUDIO2_LOOP_INFINITE : 0;

        m_sourceVoice->FlushSourceBuffers();
        m_sourceVoice->SubmitSourceBuffer(&buf);
    }

    void AudioSource::play() {
        if (!m_clip) return;

        if (!m_sourceVoice)
            createVoice();

        submitBuffer();
        m_sourceVoice->Start(0);
    }

    void AudioSource::pause() {
        if (m_sourceVoice)
            m_sourceVoice->Stop(0);
    }

    void AudioSource::stop() {
        if (!m_sourceVoice) return;
        m_sourceVoice->Stop(0);
        m_sourceVoice->FlushSourceBuffers();
    }

    void AudioSource::restart() {
        stop();
        play();
    }

    bool AudioSource::isPlaying() const {
        if (!m_sourceVoice) return false;
        XAUDIO2_VOICE_STATE state{};
        m_sourceVoice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
        return state.BuffersQueued > 0;
    }

    void AudioSource::setLoop(bool loop) {
        m_loop = loop;
    }

    void AudioSource::setVolume(float volume) {
        m_volume = std::clamp(volume, 0.f, 1.f);
        if (m_sourceVoice)
            m_sourceVoice->SetVolume(m_volume);
    }

    void AudioSource::setPitch(float pitch) {
        m_pitch = std::max(pitch, 0.001f);
        if (m_sourceVoice)
            m_sourceVoice->SetFrequencyRatio(m_pitch);
    }

    void AudioSource::setSpatial(bool spatial) {
        m_spatial = spatial;
    }

    void AudioSource::setMinDistance(float dist) {
        m_minDistance = std::max(dist, 0.001f);
    }

    void AudioSource::setMaxDistance(float dist) {
        m_maxDistance = std::max(dist, m_minDistance);
    }

    void AudioSource::updateSpatial() {
        if (!m_sourceVoice || !owner || !m_spatial) return;

        XMFLOAT3 emitterPos = getEntityPos(owner);
        XMFLOAT3 emitterFwd = getEntityForward(owner);

        XMFLOAT3 listenerPos = { 0.f, 0.f, 0.f };
        XMFLOAT3 listenerFwd = { 0.f, 0.f, 1.f };
        XMFLOAT3 listenerUp  = { 0.f, 1.f, 0.f };
        XMFLOAT3 vel         = { 0.f, 0.f, 0.f };
        XMFLOAT3 emitterUp   = { 0.f, 1.f, 0.f };

        XMVECTOR dx       = XMVectorSubtract(XMLoadFloat3(&emitterPos), XMLoadFloat3(&listenerPos));
        float    distance = XMVectorGetX(XMVector3Length(dx));

        float t     = std::clamp((distance - m_minDistance) / (m_maxDistance - m_minDistance), 0.f, 1.f);
        float atten = 1.f - t;

        X3DAUDIO_LISTENER listener{};
        listener.Position    = { listenerPos.x, listenerPos.y, listenerPos.z };
        listener.OrientFront = { listenerFwd.x, listenerFwd.y, listenerFwd.z };
        listener.OrientTop   = { listenerUp.x,  listenerUp.y,  listenerUp.z  };
        listener.Velocity    = { vel.x, vel.y, vel.z };
        listener.pCone       = nullptr;

        X3DAUDIO_EMITTER emitter{};
        emitter.Position            = { emitterPos.x, emitterPos.y, emitterPos.z };
        emitter.OrientFront         = { emitterFwd.x, emitterFwd.y, emitterFwd.z };
        emitter.OrientTop           = { emitterUp.x,  emitterUp.y,  emitterUp.z  };
        emitter.Velocity            = { vel.x, vel.y, vel.z };
        emitter.pCone               = nullptr;
        emitter.ChannelCount        = 1;
        emitter.pChannelAzimuths    = nullptr;
        emitter.CurveDistanceScaler = m_minDistance;
        emitter.DopplerScaler       = 1.f;
        emitter.InnerRadius         = 0.f;
        emitter.InnerRadiusAngle    = 0.f;
        emitter.pVolumeCurve        = nullptr;
        emitter.pLFECurve           = nullptr;
        emitter.pLPFDirectCurve     = nullptr;
        emitter.pLPFReverbCurve     = nullptr;
        emitter.pReverbCurve        = nullptr;

        uint32_t outCh = m_engine->getOutputChannels();
        std::vector<float> matrix(static_cast<size_t>(outCh), 0.f);

        X3DAUDIO_DSP_SETTINGS dsp{};
        dsp.SrcChannelCount     = 1;
        dsp.DstChannelCount     = outCh;
        dsp.pMatrixCoefficients = matrix.data();

        X3DAUDIO_HANDLE handleCopy;
        memcpy(handleCopy, m_engine->getX3DHandle(), X3DAUDIO_HANDLE_BYTESIZE);

        X3DAudioCalculate(
            handleCopy,
            &listener,
            &emitter,
            X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER,
            &dsp);

        for (float& c : matrix) c *= atten;

        m_sourceVoice->SetOutputMatrix(
            m_engine->getMasterVoice(),
            1, outCh,
            matrix.data());

        m_sourceVoice->SetFrequencyRatio(m_pitch * dsp.DopplerFactor);
    }

    void AudioSource::onUpdate(float deltaTime) {
        m_elapsed += deltaTime;

        updateSpatial();

        if (!isPlaying() && m_loop)
            play();
    }

}