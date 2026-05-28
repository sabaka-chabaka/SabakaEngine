#include "renderer/SamplerState.h"
#include "core/Logger.h"
#include <stdexcept>

namespace engine::renderer {
    static D3D11_FILTER toD3DFilter(FilterMode f) {
        switch (f) {
            case FilterMode::Point:       return D3D11_FILTER_MIN_MAG_MIP_POINT;
            case FilterMode::Bilinear:    return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            case FilterMode::Trilinear:   return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            case FilterMode::Anisotropic: return D3D11_FILTER_ANISOTROPIC;
            default:                      return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        }
    }

    static D3D11_TEXTURE_ADDRESS_MODE toD3DWrap(WrapMode w) {
        switch (w) {
            case WrapMode::Repeat: return D3D11_TEXTURE_ADDRESS_WRAP;
            case WrapMode::Clamp:  return D3D11_TEXTURE_ADDRESS_CLAMP;
            case WrapMode::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
            case WrapMode::Border: return D3D11_TEXTURE_ADDRESS_BORDER;
            default:               return D3D11_TEXTURE_ADDRESS_WRAP;
        }
    }

    SamplerState::SamplerState(ID3D11Device*        device,
                                ID3D11DeviceContext* context,
                                const SamplerDesc&   desc)
        : m_context(context)
    {
        D3D11_SAMPLER_DESC sd = {};
        sd.Filter         = toD3DFilter(desc.filter);
        sd.AddressU       = toD3DWrap(desc.wrapU);
        sd.AddressV       = toD3DWrap(desc.wrapV);
        sd.AddressW       = toD3DWrap(desc.wrapU);
        sd.MaxAnisotropy  = desc.maxAniso;
        sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sd.MinLOD         = 0.0f;
        sd.MaxLOD         = D3D11_FLOAT32_MAX;

        HRESULT hr = device->CreateSamplerState(&sd, &m_sampler);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create sampler state");
        }

        LOG_DEBUG("SamplerState created");
    }

    void SamplerState::bindPS(unsigned int slot) const {
        m_context->PSSetSamplers(slot, 1, m_sampler.GetAddressOf());
    }
    
    void SamplerState::unbindPS(unsigned int slot) const {
        ID3D11SamplerState* null = nullptr;
        m_context->PSSetSamplers(slot, 1, &null);
    }

    void SamplerState::bindVS(unsigned int slot) const {
        m_context->VSSetSamplers(slot, 1, m_sampler.GetAddressOf());
    }

    void SamplerState::unbindVS(unsigned int slot) const {
        ID3D11SamplerState* null = nullptr;
        m_context->VSSetSamplers(slot, 1, &null);
    }
}