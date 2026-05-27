#pragma once

namespace engine::renderer {

    struct BloomData {
        float threshold;
        float intensity;
        float ssaoEnabled;
        float _pad;
    };

    struct BlurData {
        float texelSizeX;
        float texelSizeY;
        int   horizontal;
        float _pad;
    };

    static_assert((sizeof(BloomData) % 16) == 0, "BloomData must be 16-byte aligned");
    static_assert((sizeof(BlurData)  % 16) == 0, "BlurData must be 16-byte aligned");

}