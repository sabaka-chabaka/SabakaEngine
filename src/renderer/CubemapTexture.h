#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <array>

namespace engine::renderer {
    using Microsoft::WRL::ComPtr;

    class CubemapTexture {
    public:
        CubemapTexture(ID3D11Device*        device,
                   ID3D11DeviceContext* context,
                   const std::array<std::string, 6>& facePaths);

        ~CubemapTexture() = default;

        CubemapTexture(const CubemapTexture&)            = delete;
        CubemapTexture& operator=(const CubemapTexture&) = delete;
        CubemapTexture(CubemapTexture&&)                 = default;
        CubemapTexture& operator=(CubemapTexture&&)      = default;

        void bindPS(unsigned int slot) const;

        ID3D11ShaderResourceView* getSRV() const;

    private:
        ComPtr<ID3D11Texture2D>          m_texture;
        ComPtr<ID3D11ShaderResourceView> m_srv;

        ID3D11DeviceContext* m_context = nullptr;
    };
}
