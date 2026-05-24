#include "ShadowMap.h"
#include "core/Logger.h"
#include <stdexcept>

namespace engine::renderer {

    ShadowMap::ShadowMap(ID3D11Device* device, const ShadowMapDesc& desc)
        : m_width(desc.width)
        , m_height(desc.height)
    {
        LOG_DEBUG("Creating shadow map " + std::to_string(m_width) + "x" + std::to_string(m_height));

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width              = m_width;
        texDesc.Height             = m_height;
        texDesc.MipLevels          = 1;
        texDesc.ArraySize          = 1;
        texDesc.Format             = DXGI_FORMAT_R32_TYPELESS;
        texDesc.SampleDesc.Count   = 1;
        texDesc.Usage              = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

        if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_texture)))
            throw std::runtime_error("ShadowMap: failed to create depth texture");

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format             = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        if (FAILED(device->CreateDepthStencilView(m_texture.Get(), &dsvDesc, &m_dsv)))
            throw std::runtime_error("ShadowMap: failed to create DSV");

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                    = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels       = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;

        if (FAILED(device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_srv)))
            throw std::runtime_error("ShadowMap: failed to create SRV");

        LOG_INFO("Shadow map created");
    }

    void ShadowMap::bindAsTarget(ID3D11DeviceContext* context) {
        ID3D11RenderTargetView* nullRTV = nullptr;
        context->OMSetRenderTargets(1, &nullRTV, m_dsv.Get());
        context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        D3D11_VIEWPORT vp = {};
        vp.Width    = static_cast<float>(m_width);
        vp.Height   = static_cast<float>(m_height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        context->RSSetViewports(1, &vp);
    }

    void ShadowMap::bindAsResource(ID3D11DeviceContext* context, uint32_t slot) {
        ID3D11ShaderResourceView* srv = m_srv.Get();
        context->PSSetShaderResources(slot, 1, &srv);
    }

    void ShadowMap::unbindAsResource(ID3D11DeviceContext* context, uint32_t slot) {
        ID3D11ShaderResourceView* null = nullptr;
        context->PSSetShaderResources(slot, 1, &null);
    }

    uint32_t ShadowMap::getWidth()  const { return m_width;  }
    uint32_t ShadowMap::getHeight() const { return m_height; }
}