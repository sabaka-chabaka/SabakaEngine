#pragma once
#include <dxgiformat.h>
#include <string>
#include <vector>
#include <wrl/client.h>

#include "GraphicsDevice.h"

namespace engine::renderer {
    using Microsoft::WRL::ComPtr;

    struct InputElementDesc {
        const char* semanticName;
        DXGI_FORMAT format;
        unsigned int offset;
    };

    class Shader {
    public:
        Shader(ID3D11Device *device,
               const std::wstring &vsPath,
               const std::wstring &psPath,
               const std::vector<InputElementDesc> &layout)

        ~Shader() = default;

        Shader(const Shader&)            = delete;
        Shader& operator=(const Shader&) = delete;

        void bind(ID3D11DeviceContext* context) const;

    private:
        std::vector<char> compileFromFile(const std::wstring& path,
                                          const std::string&  entryPoint,
                                          const std::string&  target);

        ComPtr<ID3D11VertexShader> m_vertexShader;
        ComPtr<ID3D11PixelShader>  m_pixelShader;
        ComPtr<ID3D11InputLayout>  m_inputLayout;
    };
}