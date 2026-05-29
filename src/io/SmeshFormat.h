#pragma once
#include <cstdint>

namespace engine::io {
    static constexpr char     SMESH_MAGIC[8] = {'S','M','E','S','H','\0','\0','\0'};
    static constexpr uint32_t SMESH_VERSION = 1;

    #pragma pack(push, 1)

    struct SmeshHeader {
        char magic[8];
        uint32_t version;
        uint32_t vertexCount;
        uint32_t indexCount;
        float    aabbMin[3];
        float    aabbMax[3];
    };

    #pragma pack(pop)
}