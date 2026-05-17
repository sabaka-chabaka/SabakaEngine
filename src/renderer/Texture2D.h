#pragma once
#include <string>
#include <wrl/client.h>

#include "GraphicsDevice.h"

namespace engine::renderer{
    using Microsoft::WRL::ComPtr;

    struct TextureDesc {
        bool generateMips = false;
        bool srgb = false;
    };

    class Texture2D {
    public:
        Texture2D(ID3D11Device*        device,
              ID3D11DeviceContext* context,
              const std::string&   path,
              const TextureDesc&   desc = {});

        ~Texture2D() = default;

        Texture2D(const Texture2D&)            = delete;
        Texture2D& operator=(const Texture2D&) = delete;
        Texture2D(Texture2D&&)                 = default;
        Texture2D& operator=(Texture2D&&)      = default;

        void bindPS(unsigned int slot) const;
        void bindVS(unsigned int slot) const;

        int getWidth()    const;
        int getHeight()   const;
        const std::string& getPath() const;

        ID3D11ShaderResourceView* getSRV() const;

    private:
        ComPtr<ID3D11Texture2D>           m_texture;
        ComPtr<ID3D11ShaderResourceView>  m_srv;

        ID3D11DeviceContext* m_context = nullptr;

        int         m_width  = 0;
        int         m_height = 0;
        std::string m_path;
    };
}
