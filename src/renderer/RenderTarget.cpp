#include "RenderTarget.h"
#include "core/Logger.h"
#include <stdexcept>

namespace engine::renderer {

    static DXGI_FORMAT toDXGI(RenderTargetFormat fmt) {
        switch (fmt) {
            case RenderTargetFormat::RGBA8_UNORM:  return DXGI_FORMAT_R8G8B8A8_UNORM;
            case RenderTargetFormat::RGBA16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case RenderTargetFormat::RGBA32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case RenderTargetFormat::R32_FLOAT:    return DXGI_FORMAT_R32_FLOAT;
            default:                               return DXGI_FORMAT_R16G16B16A16_FLOAT;
        }
    }

    RenderTarget::RenderTarget(ID3D11Device* device, const RenderTargetDesc& desc)
        : m_desc(desc)
    {
        LOG_DEBUG("Creating render target " + std::to_string(desc.width) + "x" + std::to_string(desc.height));
        create(device);
        LOG_INFO("Render target created");
    }

    void RenderTarget::create(ID3D11Device* device) {
        DXGI_FORMAT fmt = toDXGI(m_desc.format);

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width            = m_desc.width;
        texDesc.Height           = m_desc.height;
        texDesc.MipLevels        = 1;
        texDesc.ArraySize        = 1;
        texDesc.Format           = fmt;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage            = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags        = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_texture)))
            throw std::runtime_error("RenderTarget: failed to create texture");

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format             = fmt;
        rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        if (FAILED(device->CreateRenderTargetView(m_texture.Get(), &rtvDesc, &m_rtv)))
            throw std::runtime_error("RenderTarget: failed to create RTV");

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                    = fmt;
        srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels       = 1;

        if (FAILED(device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_srv)))
            throw std::runtime_error("RenderTarget: failed to create SRV");

        if (m_desc.hasDepth) {
            D3D11_TEXTURE2D_DESC depthDesc = {};
            depthDesc.Width            = m_desc.width;
            depthDesc.Height           = m_desc.height;
            depthDesc.MipLevels        = 1;
            depthDesc.ArraySize        = 1;
            depthDesc.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthDesc.SampleDesc.Count = 1;
            depthDesc.Usage            = D3D11_USAGE_DEFAULT;
            depthDesc.BindFlags        = D3D11_BIND_DEPTH_STENCIL;

            if (FAILED(device->CreateTexture2D(&depthDesc, nullptr, &m_depthTexture)))
                throw std::runtime_error("RenderTarget: failed to create depth texture");

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
            dsvDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;

            if (FAILED(device->CreateDepthStencilView(m_depthTexture.Get(), &dsvDesc, &m_dsv)))
                throw std::runtime_error("RenderTarget: failed to create DSV");
        }
    }

    void RenderTarget::release() {
        m_rtv.Reset(); m_srv.Reset(); m_texture.Reset();
        m_dsv.Reset(); m_depthTexture.Reset();
    }

    void RenderTarget::bindSRV(ID3D11DeviceContext* context, uint32_t slot) {
        ID3D11ShaderResourceView* srv = m_srv.Get();
        context->PSSetShaderResources(slot, 1, &srv);
    }

    void RenderTarget::unbindSRV(ID3D11DeviceContext* context, uint32_t slot) {
        ID3D11ShaderResourceView* null = nullptr;
        context->PSSetShaderResources(slot, 1, &null);
    }

    void RenderTarget::resize(ID3D11Device* device, uint32_t width, uint32_t height) {
        LOG_DEBUG("Resizing render target to " + std::to_string(width) + "x" + std::to_string(height));
        m_desc.width  = width;
        m_desc.height = height;
        release();
        create(device);
    }

    uint32_t                  RenderTarget::getWidth()  const { return m_desc.width; }
    uint32_t                  RenderTarget::getHeight() const { return m_desc.height; }
    ID3D11RenderTargetView*   RenderTarget::getRTV()    const { return m_rtv.Get(); }
    ID3D11DepthStencilView*   RenderTarget::getDSV()    const { return m_dsv.Get(); }
    ID3D11ShaderResourceView* RenderTarget::getSRV()    const { return m_srv.Get(); }
}