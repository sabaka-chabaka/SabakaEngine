#pragma once

namespace engine::renderer {

    struct DofData {
        float focusDistance;
        float focusRange;
        float maxBlurRadius;
        int   enabled;
        float nearZ;
        float farZ;
        int   numSamples;
        float _pad;
    };

    static_assert((sizeof(DofData) % 16) == 0, "DofData must be 16-byte aligned");

}