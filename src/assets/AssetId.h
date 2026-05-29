#pragma once
#include <cstdint>
#include <functional>

namespace engine::asset {

    using AssetId = uint64_t;

    static constexpr AssetId INVALID_ASSET_ID = 0;

    inline bool isValid(AssetId id) {
        return id != INVALID_ASSET_ID;
    }

}

namespace std {
    template<>
    struct hash<engine::asset::AssetId> {
        size_t operator()(engine::asset::AssetId id) const noexcept {
            return std::hash<uint64_t>{}(id);
        }
    };
}