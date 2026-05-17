#pragma once
#include "DirectXMath.h"

namespace engine::renderer {
    struct alignas(16) MaterialData {
        float specularIntensity = 1.0f;
        float specularPower     = 32.0f;
        DirectX::XMFLOAT2 uvScale  = { 1.0f, 1.0f };
        DirectX::XMFLOAT2 uvOffset = { 0.0f, 0.0f };
        DirectX::XMFLOAT2 _pad     = { 0.0f, 0.0f };
    };

    static_assert(sizeof(MaterialData) % 16 == 0,
        "MaterialData must be 16-byte aligned");
}