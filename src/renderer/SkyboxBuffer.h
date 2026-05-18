#pragma once
#include <DirectXMath.h>

namespace engine::renderer {
    struct alignas(16) SkyboxData {
        DirectX::XMMATRIX invViewProj;
        DirectX::XMMATRIX dummy;
    };

    static_assert(sizeof(SkyboxData) % 16 == 0,
        "SkyboxData must be 16-byte aligned");
}