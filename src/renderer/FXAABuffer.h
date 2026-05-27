#pragma once

namespace engine::renderer {

    struct FXAAData {
        float texelSizeX;
        float texelSizeY;
        int   enabled;
        float _pad;
    };

    static_assert((sizeof(FXAAData) % 16) == 0,
        "FXAAData must be 16-byte aligned");

}
