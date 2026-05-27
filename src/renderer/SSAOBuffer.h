#pragma once
#include <DirectXMath.h>
#include <array>
#include <random>
#include <cmath>

namespace engine::renderer {

    static constexpr int SSAO_NUM_SAMPLES = 32;
    static constexpr int SSAO_NOISE_SIZE  = 4;

    struct alignas(16) SSAOData {
        DirectX::XMFLOAT4 samples[64];
        DirectX::XMMATRIX projection;
        DirectX::XMMATRIX invProjection;
        float             noiseScaleX;
        float             noiseScaleY;
        float             radius;
        float             bias;
        int               numSamples;
        float             _pad[3];
    };

    static_assert((sizeof(SSAOData) % 16) == 0, "SSAOData must be 16-byte aligned");

    struct SSAOBlurData {
        float texelSizeX;
        float texelSizeY;
        float depthThreshold;
        float _pad;
    };

    static_assert((sizeof(SSAOBlurData) % 16) == 0, "SSAOBlurData must be 16-byte aligned");

    inline std::array<DirectX::XMFLOAT4, 64> generateSSAOSamples() {
        std::uniform_real_distribution<float> rndFloat(0.0f, 1.0f);
        std::uniform_real_distribution<float> rndSigned(-1.0f, 1.0f);
        std::default_random_engine            rng(42u);

        std::array<DirectX::XMFLOAT4, 64> samples;
        for (int i = 0; i < 64; ++i) {
            DirectX::XMVECTOR s = DirectX::XMVectorSet(
                rndSigned(rng),
                rndSigned(rng),
                rndFloat(rng),
                0.0f
            );
            s = DirectX::XMVector3Normalize(s);
            s = DirectX::XMVectorScale(s, rndFloat(rng));

            float scale = static_cast<float>(i) / 64.0f;
            scale = 0.1f + scale * scale * 0.9f;
            s = DirectX::XMVectorScale(s, scale);

            DirectX::XMStoreFloat4(&samples[i], s);
        }
        return samples;
    }

    inline std::array<DirectX::XMFLOAT4, SSAO_NOISE_SIZE * SSAO_NOISE_SIZE> generateSSAONoise() {
        std::uniform_real_distribution<float> rndSigned(-1.0f, 1.0f);
        std::default_random_engine            rng(7u);

        std::array<DirectX::XMFLOAT4, SSAO_NOISE_SIZE * SSAO_NOISE_SIZE> noise;
        for (auto& n : noise) {
            n = { rndSigned(rng), rndSigned(rng), 0.0f, 0.0f };
        }
        return noise;
    }
}