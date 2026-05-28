#pragma once
#include <d3d11.h>
#include <wrl/client.h>

namespace engine::renderer {
    using Microsoft::WRL::ComPtr;

    enum class FilterMode {
        Point,
        Bilinear,
        Trilinear,
        Anisotropic
    };

    enum class WrapMode {
        Repeat,
        Clamp,
        Mirror,
        Border,
    };

    struct SamplerDesc {
        FilterMode filter       = FilterMode::Bilinear;
        WrapMode   wrapU        = WrapMode::Repeat;
        WrapMode   wrapV        = WrapMode::Repeat;
        unsigned int maxAniso   = 4;
    };

    class SamplerState {
    public:
        SamplerState(ID3D11Device*       device,
                     ID3D11DeviceContext* context,
                     const SamplerDesc&   desc = {});

        ~SamplerState() = default;

        SamplerState(const SamplerState&)            = delete;
        SamplerState& operator=(const SamplerState&) = delete;

        void bindPS(unsigned int slot) const;
        void unbindPS(unsigned int slot) const;
        void bindVS(unsigned int slot) const;
        void unbindVS(unsigned int slot) const;

    private:
        ComPtr<ID3D11SamplerState> m_sampler;
        ID3D11DeviceContext*       m_context = nullptr;
    };
}