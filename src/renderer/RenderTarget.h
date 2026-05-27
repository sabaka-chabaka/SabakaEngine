#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <cstdint>

namespace engine::renderer {
    using Microsoft::WRL::ComPtr;

    enum class RenderTargetFormat {
        RGBA8_UNORM,
        RGBA16_FLOAT,
        RGBA32_FLOAT,
        R32_FLOAT,
    };

    struct RenderTargetDesc {
        uint32_t           width         = 1280;
        uint32_t           height        = 720;
        RenderTargetFormat format        = RenderTargetFormat::RGBA16_FLOAT;
        bool               hasDepth      = false;
        bool               readableDepth = false;
        uint32_t           sampleCount   = 1;
    };

    class RenderTarget {
    public:
        RenderTarget(ID3D11Device* device, const RenderTargetDesc& desc);
        ~RenderTarget() = default;

        RenderTarget(const RenderTarget&)            = delete;
        RenderTarget& operator=(const RenderTarget&) = delete;

        void bindSRV(ID3D11DeviceContext* context, uint32_t slot);
        void unbindSRV(ID3D11DeviceContext* context, uint32_t slot);
        void bindDepthSRV(ID3D11DeviceContext* context, uint32_t slot);
        void unbindDepthSRV(ID3D11DeviceContext* context, uint32_t slot);

        void resolveInto(ID3D11DeviceContext* context, RenderTarget* dest);

        void resize(ID3D11Device* device, uint32_t width, uint32_t height);

        uint32_t getWidth()      const;
        uint32_t getHeight()     const;
        bool     isMSAA()        const;

        ID3D11RenderTargetView*   getRTV() const;
        ID3D11DepthStencilView*   getDSV() const;
        ID3D11ShaderResourceView* getSRV() const;
        void                      captureToImage(ID3D11DeviceContext* context, const char* filename);

    private:
        void create(ID3D11Device* device);
        void release();

        RenderTargetDesc                 m_desc;
        ComPtr<ID3D11Texture2D>          m_texture;
        ComPtr<ID3D11RenderTargetView>   m_rtv;
        ComPtr<ID3D11ShaderResourceView> m_srv;
        ComPtr<ID3D11Texture2D>          m_depthTexture;
        ComPtr<ID3D11DepthStencilView>   m_dsv;
        ComPtr<ID3D11ShaderResourceView> m_depthSRV;
    };
}