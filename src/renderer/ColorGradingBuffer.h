#pragma once

namespace engine::renderer {

    struct ColorGradingData {
        float lutStrength;
        int   lutEnabled;
        float _pad0;
        float _pad1;
    };

    static_assert((sizeof(ColorGradingData) % 16) == 0, "ColorGradingData must be 16-byte aligned");

    struct VignetteData {
        float innerRadius;
        float outerRadius;
        float intensity;
        int   vignetteEnabled;
        float aberrationStrength;
        int   aberrationEnabled;
        float _pad0;
        float _pad1;
    };

    static_assert((sizeof(VignetteData) % 16) == 0, "VignetteData must be 16-byte aligned");

}