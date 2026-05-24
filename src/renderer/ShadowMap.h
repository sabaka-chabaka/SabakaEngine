#pragma once
#include <cstdint>
#include <wrl/client.h>

#include "GraphicsDevice.h"

namespace engine::renderer {
    using Microsoft::WRL::ComPtr;

    struct ShadowMapDesc {
        uint32_t width = 2048;
        uint32_t height = 2048;
    };

    class ShadowMap {
    public:
        ShadowMap(ID3D11Device* device, const ShadowMapDesc& desc);
        ~ShadowMap() = default;

        ShadowMap(const ShadowMap&)            = delete;
        ShadowMap& operator=(const ShadowMap&) = delete;

        void bindAsTarget(ID3D11DeviceContext* context);
        void bindAsResource(ID3D11DeviceContext* context, uint32_t slot);
        void unbindAsResource(ID3D11DeviceContext* context, uint32_t slot);

        uint32_t getWidth()  const;
        uint32_t getHeight() const;

    private:
        ComPtr<ID3D11Texture2D>          m_texture;
        ComPtr<ID3D11DepthStencilView>   m_dsv;
        ComPtr<ID3D11ShaderResourceView> m_srv;

        uint32_t m_width;
        uint32_t m_height;
    };
}