#include "renderer/RenderTarget.h"
#include "core/Logger.h"
#include <stdexcept>
#include <fstream>
#include <vector>
#include <algorithm>

namespace engine::renderer {

    static void saveBMP(const char* filename, int width, int height, const uint8_t* rgba) {
        std::ofstream f(filename, std::ios::binary);
        if (!f) return;

        uint32_t fileSize = 54 + width * height * 3;
        uint8_t header[54] = {
            'B', 'M',
            (uint8_t)(fileSize), (uint8_t)(fileSize >> 8), (uint8_t)(fileSize >> 16), (uint8_t)(fileSize >> 24),
            0, 0, 0, 0,
            54, 0, 0, 0,
            40, 0, 0, 0,
            (uint8_t)(width), (uint8_t)(width >> 8), (uint8_t)(width >> 16), (uint8_t)(width >> 24),
            (uint8_t)(height), (uint8_t)(height >> 8), (uint8_t)(height >> 16), (uint8_t)(height >> 24),
            1, 0,
            24, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0
        };

        f.write((char*)header, 54);
        for (int y = height - 1; y >= 0; --y) {
            for (int x = 0; x < width; ++x) {
                const uint8_t* p = &rgba[(y * width + x) * 4];
                f.put(p[2]); // B
                f.put(p[1]); // G
                f.put(p[0]); // R
            }
            int padding = (4 - (width * 3) % 4) % 4;
            for (int i = 0; i < padding; ++i) f.put(0);
        }
    }

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

        UINT sampleCount   = m_desc.sampleCount > 1 ? m_desc.sampleCount : 1;
        UINT sampleQuality = 0;
        if (sampleCount > 1) {
            UINT levels = 0;
            device->CheckMultisampleQualityLevels(fmt, sampleCount, &levels);
            if (levels == 0) {
                LOG_DEBUG("MSAA x" + std::to_string(sampleCount) + " not supported for this format, falling back to 1x");
                sampleCount = 1;
            } else {
                sampleQuality = levels - 1;
            }
        }

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width              = m_desc.width;
        texDesc.Height             = m_desc.height;
        texDesc.MipLevels          = 1;
        texDesc.ArraySize          = 1;
        texDesc.Format             = fmt;
        texDesc.SampleDesc.Count   = sampleCount;
        texDesc.SampleDesc.Quality = sampleQuality;
        texDesc.Usage              = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags          = D3D11_BIND_RENDER_TARGET;
        if (sampleCount == 1)
            texDesc.BindFlags     |= D3D11_BIND_SHADER_RESOURCE;

        if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_texture)))
            throw std::runtime_error("RenderTarget: failed to create texture");

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format        = fmt;
        rtvDesc.ViewDimension = (sampleCount > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS
                                                  : D3D11_RTV_DIMENSION_TEXTURE2D;
        if (sampleCount == 1)
            rtvDesc.Texture2D.MipSlice = 0;

        if (FAILED(device->CreateRenderTargetView(m_texture.Get(), &rtvDesc, &m_rtv)))
            throw std::runtime_error("RenderTarget: failed to create RTV");

        if (sampleCount == 1) {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format                    = fmt;
            srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels       = 1;

            if (FAILED(device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_srv)))
                throw std::runtime_error("RenderTarget: failed to create SRV");
        }

        if (m_desc.hasDepth) {
            D3D11_TEXTURE2D_DESC depthDesc = {};
            depthDesc.Width              = m_desc.width;
            depthDesc.Height             = m_desc.height;
            depthDesc.MipLevels          = 1;
            depthDesc.ArraySize          = 1;
            depthDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthDesc.SampleDesc.Count   = sampleCount;
            depthDesc.SampleDesc.Quality = sampleQuality;
            depthDesc.Usage              = D3D11_USAGE_DEFAULT;
            depthDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;

            if (FAILED(device->CreateTexture2D(&depthDesc, nullptr, &m_depthTexture)))
                throw std::runtime_error("RenderTarget: failed to create depth texture");

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format        = DXGI_FORMAT_D24_UNORM_S8_UINT;
            dsvDesc.ViewDimension = (sampleCount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS
                                                      : D3D11_DSV_DIMENSION_TEXTURE2D;
            if (sampleCount == 1)
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

    void RenderTarget::resolveInto(ID3D11DeviceContext* context, RenderTarget* dest) {
        DXGI_FORMAT fmt = toDXGI(m_desc.format);
        context->ResolveSubresource(dest->m_texture.Get(), 0, m_texture.Get(), 0, fmt);
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
    bool                      RenderTarget::isMSAA()    const { return m_desc.sampleCount > 1; }
    ID3D11RenderTargetView*   RenderTarget::getRTV()    const { return m_rtv.Get(); }
    ID3D11DepthStencilView*   RenderTarget::getDSV()    const { return m_dsv.Get(); }
    ID3D11ShaderResourceView* RenderTarget::getSRV()    const { return m_srv.Get(); }

    void RenderTarget::captureToImage(ID3D11DeviceContext* context, const char* filename) {
        D3D11_TEXTURE2D_DESC desc;
        m_texture->GetDesc(&desc);

        D3D11_TEXTURE2D_DESC stagingDesc = desc;
        stagingDesc.Usage = D3D11_USAGE_STAGING;
        stagingDesc.BindFlags = 0;
        stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        stagingDesc.MiscFlags = 0;

        ComPtr<ID3D11Device> device;
        context->GetDevice(&device);

        ComPtr<ID3D11Texture2D> stagingTexture;
        if (FAILED(device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture))) return;

        context->CopyResource(stagingTexture.Get(), m_texture.Get());

        D3D11_MAPPED_SUBRESOURCE mapped;
        if (FAILED(context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped))) return;

        std::vector<uint8_t> rgba(desc.Width * desc.Height * 4);
        const uint8_t* src = (const uint8_t*)mapped.pData;

        if (desc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT) {
            for (uint32_t y = 0; y < desc.Height; ++y) {
                for (uint32_t x = 0; x < desc.Width; ++x) {
                    const uint16_t* p = (const uint16_t*)(src + y * mapped.RowPitch + x * 8);
                    auto h2f = [](uint16_t h) {
                        uint32_t s = (h >> 15) & 0x0001;
                        uint32_t e = (h >> 10) & 0x001f;
                        uint32_t m = h & 0x03ff;
                        if (e == 0) return 0.0f;
                        if (e == 31) return 1.0f;
                        return (s ? -1.0f : 1.0f) * powf(2.0f, (float)e - 15.0f) * (1.0f + (float)m / 1024.0f);
                    };
                    rgba[(y * desc.Width + x) * 4 + 0] = (uint8_t)(std::clamp(h2f(p[0]), 0.0f, 1.0f) * 255.0f);
                    rgba[(y * desc.Width + x) * 4 + 1] = (uint8_t)(std::clamp(h2f(p[1]), 0.0f, 1.0f) * 255.0f);
                    rgba[(y * desc.Width + x) * 4 + 2] = (uint8_t)(std::clamp(h2f(p[2]), 0.0f, 1.0f) * 255.0f);
                    rgba[(y * desc.Width + x) * 4 + 3] = 255;
                }
            }
        } else if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM) {
            for (uint32_t y = 0; y < desc.Height; ++y) {
                memcpy(&rgba[y * desc.Width * 4], src + y * mapped.RowPitch, desc.Width * 4);
            }
        }

        context->Unmap(stagingTexture.Get(), 0);
        saveBMP(filename, desc.Width, desc.Height, rgba.data());
    }
}