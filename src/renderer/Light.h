#pragma once
#include <DirectXMath.h>

namespace engine::renderer {

    struct alignas(16) LightData {
        DirectX::XMFLOAT4 ambientColor   = { 0.2f, 0.2f, 0.2f, 1.0f };
        DirectX::XMFLOAT3 lightDirection = { 0.0f, -1.0f, 1.0f };
        float             padding;
        DirectX::XMFLOAT4 lightColor     = { 1.0f, 1.0f, 1.0f, 1.0f };
    };

    static_assert(sizeof(LightData) % 16 == 0, "LightData must be 16-byte aligned");

}
