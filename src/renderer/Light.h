#pragma once
#include <DirectXMath.h>

namespace engine::renderer {

    struct alignas(16) LightData {
        DirectX::XMFLOAT4 ambientColor    = { 0.2f, 0.2f, 0.2f, 1.0f };
        DirectX::XMFLOAT3 lightDirection  = { 0.0f, -1.0f, 1.0f };
        float             _pad0           = 0.0f;
        DirectX::XMFLOAT4 lightColor      = { 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT3 viewPos         = { 0.0f, 0.0f, 0.0f };
        float             _pad1           = 0.0f;

        DirectX::XMFLOAT4 pointLightPos   = { 0.0f, 1.5f, 0.0f, 1.0f };
        DirectX::XMFLOAT4 pointLightColor = { 1.0f, 0.4f, 0.1f, 1.0f };

        float attConstant  = 1.0f;
        float attLinear    = 0.22f;
        float attQuadratic = 0.20f;
        float pointEnabled = 1.0f;

        DirectX::XMFLOAT4 spotLightPos   = {  0.0f, 4.0f,  0.0f, 1.0f };
        DirectX::XMFLOAT4 spotLightDir   = {  0.0f, -1.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT4 spotLightColor = {  0.2f, 0.6f,  1.0f, 1.0f };

        float spotCosInner = 0.9848f;
        float spotCosOuter = 0.9397f;
        float spotEnabled  = 1.0f;
        float _spotPad     = 0.0f;
    };

    static_assert(sizeof(LightData) % 16 == 0, "LightData must be 16-byte aligned");

}
