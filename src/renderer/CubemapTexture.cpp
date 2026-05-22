#include "CubemapTexture.h"
#include "core/Logger.h"
#include "../../third_party/stb_image/stb_image.h"
#include <stdexcept>
#include <vector>

namespace engine::renderer {
    CubemapTexture::CubemapTexture(ID3D11Device *device,
                                   ID3D11DeviceContext *context,
                                   const std::array<std::string, 6> &facePaths)
        : m_context(context) {
        stbi_set_flip_vertically_on_load(0);

        struct FaceData {
            unsigned char *pixels = nullptr;
            int width = 0;
            int height = 0;
        };

        std::array<FaceData, 6> faces;
        int faceSize = 0;

        for (int i = 0; i < 6; ++i) {
            int channels = 0;
            faces[i].pixels = stbi_load(
                facePaths[i].c_str(),
                &faces[i].width,
                &faces[i].height,
                &channels,
                4
            );

            if (!faces[i].pixels) {
                for (int j = 0; j < i; ++j) stbi_image_free(faces[j].pixels);
                throw std::runtime_error("Failed to load cubemap face: " + facePaths[i]);
            }

            if (faces[i].width != faces[i].height) {
                for (int j = 0; j <= i; ++j) stbi_image_free(faces[j].pixels);
                throw std::runtime_error("Cubemap face must be square: " + facePaths[i]);
            }

            if (i == 0) {
                faceSize = faces[i].width;
            } else if (faces[i].width != faceSize) {
                for (int j = 0; j <= i; ++j) stbi_image_free(faces[j].pixels);
                throw std::runtime_error("All cubemap faces must be the same size");
            }

            LOG_DEBUG("Cubemap face [" + std::to_string(i) + "] loaded: " + facePaths[i]);
        }

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = static_cast<UINT>(faceSize);
        texDesc.Height = static_cast<UINT>(faceSize);
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 6;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_IMMUTABLE;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        UINT rowPitch = static_cast<UINT>(faceSize) * 4;

        std::array<D3D11_SUBRESOURCE_DATA, 6> initData = {};
        for (int i = 0; i < 6; ++i) {
            initData[i].pSysMem = faces[i].pixels;
            initData[i].SysMemPitch = rowPitch;
        }

        HRESULT hr = device->CreateTexture2D(&texDesc, initData.data(), &m_texture);

        for (int i = 0; i < 6; ++i) stbi_image_free(faces[i].pixels);

        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create cubemap texture");
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip = 0;
        srvDesc.TextureCube.MipLevels = 1;

        hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_srv);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create cubemap SRV");
        }

        LOG_INFO("Cubemap loaded: " + std::to_string(faceSize) + "x" +
                 std::to_string(faceSize) + " per face");
    }

    void CubemapTexture::bindPS(unsigned int slot) const {
        m_context->PSSetShaderResources(slot, 1, m_srv.GetAddressOf());
    }

    ID3D11ShaderResourceView *CubemapTexture::getSRV() const {
        return m_srv.Get();
    }
}
