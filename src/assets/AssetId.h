#pragma once
#include <cstdint>
#include <functional>

namespace engine::assets {

    using AssetId = uint64_t;

    static constexpr AssetId INVALID_ASSET_ID = 0;

    inline bool isValid(AssetId id) {
        return id != INVALID_ASSET_ID;
    }

}