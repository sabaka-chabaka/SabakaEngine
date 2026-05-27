#pragma once
#include <DirectXMath.h>

namespace engine::renderer {
    enum class TonemapMode : int {
        Reinhard = 0,
        ACES = 1
    };

    struct PostProcessData {
        float exposure;
        int tonemapMode;
        float _pad0;
        float _pad1;
    };

    static_assert((sizeof(PostProcessData) % 16) == 0, "PostProcessData must be 16-byte aligned");
}