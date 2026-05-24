#include "renderer/ShadowSampler.h"
#include "core/Logger.h"
#include <stdexcept>

namespace engine::renderer {

    ShadowSampler::ShadowSampler(ID3D11Device* device) {
        LOG_DEBUG("Creating shadow comparison sampler...");

        D3D11_SAMPLER_DESC desc = {};
        desc.Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        desc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
        desc.MinLOD         = 0.0f;
        desc.MaxLOD         = D3D11_FLOAT32_MAX;

        if (FAILED(device->CreateSamplerState(&desc, &m_sampler)))
            throw std::runtime_error("ShadowSampler: failed to create sampler state");

        LOG_INFO("Shadow sampler created");
    }

    void ShadowSampler::bindPS(ID3D11DeviceContext* context, uint32_t slot) {
        ID3D11SamplerState* s = m_sampler.Get();
        context->PSSetSamplers(slot, 1, &s);
    }
}