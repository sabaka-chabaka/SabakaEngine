#pragma once
#include <DirectXMath.h>

namespace engine::renderer {

    struct alignas(16) LightData {
        DirectX::XMFLOAT4 ambientColor = { 0.2f, 0.2f, 0.2f, 1.0f };
    };

    static_assert(sizeof(LightData) % 16 == 0, "LightData must be 16-byte aligned");

}
