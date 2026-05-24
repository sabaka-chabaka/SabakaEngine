#pragma once
#include <cstdint>
#include <wrl/client.h>

#include "GraphicsDevice.h"

namespace engine::renderer {
    using Microsoft::WRL::ComPtr;

    class ShadowSampler {
    public:
        explicit ShadowSampler(ID3D11Device* device);
        ~ShadowSampler() = default;

        ShadowSampler(const ShadowSampler&)            = delete;
        ShadowSampler& operator=(const ShadowSampler&) = delete;

        void bindPS(ID3D11DeviceContext* context, uint32_t slot);

    private:
        ComPtr<ID3D11SamplerState> m_sampler;
    };
}
