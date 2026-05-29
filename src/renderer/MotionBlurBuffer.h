#pragma once
#include <DirectXMath.h>

namespace engine::renderer {
    struct alignas(16) MotionBlurData {
        DirectX::XMMATRIX prevViewProj;
        DirectX::XMMATRIX invViewProj;
        float strength;
        int numSamples;
        int enabled;
        float _pad;
    };

    static_assert(sizeof(MotionBlurData) % 16 == 0,
        "MotionBlurData must be 16-byte aligned");
}