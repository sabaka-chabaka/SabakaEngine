#include "renderer/Shader.h"
#include "core/Logger.h"
#include <d3dcompiler.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iterator>

namespace engine::renderer {

Shader::Shader(ID3D11Device*                        device,
               const std::wstring&                  vsPath,
               const std::wstring&                  psPath,
               const std::vector<InputElementDesc>&  layout)
{
    std::string vsPathString(vsPath.begin(), vsPath.end());
    std::string psPathString(psPath.begin(), psPath.end());
    LOG_DEBUG("Compiling shader: VS=" + vsPathString + ", PS=" + psPathString);

    std::vector<char> vsBytecode = compileFromFile(vsPath, "main", "vs_5_0");
    std::vector<char> psBytecode = compileFromFile(psPath, "main", "ps_5_0");

    HRESULT hr = device->CreateVertexShader(
        vsBytecode.data(),
        vsBytecode.size(),
        nullptr,
        &m_vertexShader
    );
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create vertex shader");
        throw std::runtime_error("Failed to create vertex shader");
    }
    LOG_DEBUG("Vertex shader created");

    hr = device->CreatePixelShader(
        psBytecode.data(),
        psBytecode.size(),
        nullptr,
        &m_pixelShader
    );
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create pixel shader");
        throw std::runtime_error("Failed to create pixel shader");
    }
    LOG_DEBUG("Pixel shader created");

    std::vector<D3D11_INPUT_ELEMENT_DESC> d3dLayout;
    d3dLayout.reserve(layout.size());

    for (const auto& elem : layout) {
        D3D11_INPUT_ELEMENT_DESC d3dElem = {};
        d3dElem.SemanticName             = elem.semanticName;
        d3dElem.SemanticIndex            = 0;
        d3dElem.Format                   = elem.format;
        d3dElem.InputSlot                = 0;
        d3dElem.AlignedByteOffset        = elem.offset;
        d3dElem.InputSlotClass           = D3D11_INPUT_PER_VERTEX_DATA;
        d3dElem.InstanceDataStepRate     = 0;
        d3dLayout.push_back(d3dElem);
    }

    if (!d3dLayout.empty()) {
        hr = device->CreateInputLayout(
            d3dLayout.data(),
            static_cast<UINT>(d3dLayout.size()),
            vsBytecode.data(),
            vsBytecode.size(),
            &m_inputLayout
        );
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create input layout");
        }
    }

    LOG_INFO("Shader compiled and linked successfully: VS=" + vsPathString + ", PS=" + psPathString);
}

void Shader::bind(ID3D11DeviceContext* context) const {
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(),  nullptr, 0);
    context->IASetInputLayout(m_inputLayout.Get());
}

    std::vector<char> Shader::compileFromFile(const std::wstring& path,
                                               const std::string&  entryPoint,
                                               const std::string&  target)
{
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    std::filesystem::path absolutePath =
        std::filesystem::path(exePath).parent_path() / path;

    LOG_DEBUG("Loading shader: " + absolutePath.string());

    UINT compileFlags = 0;
#ifdef _DEBUG
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompileFromFile(
        absolutePath.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.c_str(),
        target.c_str(),
        compileFlags,
        0,
        &shaderBlob,
        &errorBlob
    );

    if (FAILED(hr)) {
        std::string errorMsg = "Shader compilation failed: ";
        if (errorBlob) {
            errorMsg += static_cast<const char*>(errorBlob->GetBufferPointer());
        }
        LOG_ERROR(errorMsg);
        throw std::runtime_error(errorMsg);
    }

    const char* begin = static_cast<const char*>(shaderBlob->GetBufferPointer());
    const char* end   = begin + shaderBlob->GetBufferSize();
    return std::vector<char>(begin, end);
}

}