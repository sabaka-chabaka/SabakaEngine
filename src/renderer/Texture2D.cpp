#define STB_IMAGE_IMPLEMENTATION
#include "../../third_party/stb_image/stb_image.h"

#include "Texture2D.h"
#include "core/Logger.h"
#include <stdexcept>
#include <filesystem>

namespace engine::renderer {
    Texture2D::Texture2D(ID3D11Device *device,
                         ID3D11DeviceContext *context,
                         const std::string &path,
                         const TextureDesc &desc)
        : m_context(context)
          , m_path(path) {
        stbi_set_flip_vertically_on_load(0);

        int channels = 0;
        unsigned char *data = stbi_load(path.c_str(), &m_width, &m_height, &channels, 4);

        if (!data) {
            std::string err = "Failed to load texture: " + path;
            if (stbi_failure_reason()) {
                err += " — ";
                err += stbi_failure_reason();
            }
            throw std::runtime_error(err);
        }

        DXGI_FORMAT format = desc.srgb
                                 ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
                                 : DXGI_FORMAT_R8G8B8A8_UNORM;

        UINT mipLevels = 1;
        if (desc.generateMips) {
            UINT maxDim = static_cast<UINT>(max(m_width, m_height));
            mipLevels = 1 + static_cast<UINT>(std::floor(std::log2(maxDim)));
        }

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = static_cast<UINT>(m_width);
        texDesc.Height = static_cast<UINT>(m_height);
        texDesc.MipLevels = mipLevels;
        texDesc.ArraySize = 1;
        texDesc.Format = format;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = desc.generateMips
                            ? D3D11_USAGE_DEFAULT
                            : D3D11_USAGE_IMMUTABLE;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        if (desc.generateMips) {
            texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
            texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }

        HRESULT hr;

        if (desc.generateMips) {
            hr = device->CreateTexture2D(&texDesc, nullptr, &m_texture);
            if (FAILED(hr)) {
                stbi_image_free(data);
                throw std::runtime_error("Failed to create texture2D for mip gen");
            }

            UINT rowPitch = static_cast<UINT>(m_width) * 4;
            context->UpdateSubresource(m_texture.Get(), 0, nullptr, data, rowPitch, 0);
        } else {
            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = data;
            initData.SysMemPitch = static_cast<UINT>(m_width) * 4;

            hr = device->CreateTexture2D(&texDesc, &initData, &m_texture);
            if (FAILED(hr)) {
                stbi_image_free(data);
                throw std::runtime_error("Failed to create texture2D: " + path);
            }
        }

        stbi_image_free(data);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = mipLevels;

        hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_srv);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create SRV: " + path);
        }

        if (desc.generateMips) {
            context->GenerateMips(m_srv.Get());
        }

        LOG_INFO("Texture loaded: " + path +
            " (" + std::to_string(m_width) +
            "x" + std::to_string(m_height) +
            ", mips=" + std::to_string(mipLevels) + ")");
    }

    Texture2D::Texture2D(ID3D11Device*        device,
                         ID3D11DeviceContext* context,
                         int                  width,
                         int                  height,
                         const unsigned char* rgba,
                         const TextureDesc&   desc)
        : m_context(context)
          , m_width(width)
          , m_height(height)
          , m_path("<raw>")
    {
        DXGI_FORMAT format = desc.srgb
            ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
            : DXGI_FORMAT_R8G8B8A8_UNORM;

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width              = static_cast<UINT>(width);
        texDesc.Height             = static_cast<UINT>(height);
        texDesc.MipLevels          = 1;
        texDesc.ArraySize          = 1;
        texDesc.Format             = format;
        texDesc.SampleDesc.Count   = 1;
        texDesc.Usage              = D3D11_USAGE_IMMUTABLE;
        texDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags     = 0;
        texDesc.MiscFlags          = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem     = rgba;
        initData.SysMemPitch  = static_cast<UINT>(width) * 4;

        HRESULT hr = device->CreateTexture2D(&texDesc, &initData, &m_texture);
        if (FAILED(hr)) throw std::runtime_error("Failed to create raw Texture2D");

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                    = format;
        srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels       = 1;

        hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_srv);
        if (FAILED(hr)) throw std::runtime_error("Failed to create SRV for raw texture");

        LOG_DEBUG("Raw texture created: " + std::to_string(width) + "x" + std::to_string(height));
    }

    void Texture2D::bindPS(unsigned int slot) const {
        m_context->PSSetShaderResources(slot, 1, m_srv.GetAddressOf());
    }

    void Texture2D::bindVS(unsigned int slot) const {
        m_context->VSSetShaderResources(slot, 1, m_srv.GetAddressOf());
    }

    int Texture2D::getWidth() const { return m_width; }
    int Texture2D::getHeight() const { return m_height; }
    const std::string &Texture2D::getPath() const { return m_path; }

    ID3D11ShaderResourceView *Texture2D::getSRV() const {
        return m_srv.Get();
    }
}
