#include "io/SmeshLoader.h"
#include "io/SmeshFormat.h"
#include "core/Logger.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace engine::io {
    bool SmeshLoader::isValid(const std::string& path)
    {
        FILE* f = fopen(path.c_str(), "rb");
        if (!f) return false;

        SmeshHeader hdr{};
        bool ok = fread(&hdr, sizeof(hdr), 1, f) == 1
               && memcmp(hdr.magic, SMESH_MAGIC, 8) == 0
               && hdr.version == SMESH_VERSION
               && hdr.vertexCount > 0
               && hdr.indexCount  > 0;

        fclose(f);
        return ok;
    }

    renderer::Mesh SmeshLoader::load(
        ID3D11Device*        device,
        ID3D11DeviceContext* context,
        const std::string&   path)
    {
        FILE* f = fopen(path.c_str(), "rb");
        if (!f) {
            LOG_ERROR("SmeshLoader: cannot open " + path);
            throw std::runtime_error("SmeshLoader: cannot open " + path);
        }

        SmeshHeader hdr{};
        if (fread(&hdr, sizeof(hdr), 1, f) != 1
            || memcmp(hdr.magic, SMESH_MAGIC, 8) != 0
            || hdr.version != SMESH_VERSION) {
            fclose(f);
            throw std::runtime_error("SmeshLoader: invalid header in " + path);
        }

        std::vector<renderer::Vertex> vertices(hdr.vertexCount);
        std::vector<uint32_t>         indices(hdr.indexCount);

        if (fread(vertices.data(), sizeof(renderer::Vertex), hdr.vertexCount, f) != hdr.vertexCount) {
            fclose(f);
            throw std::runtime_error("SmeshLoader: vertex read error in " + path);
        }

        if (fread(indices.data(), sizeof(uint32_t), hdr.indexCount, f) != hdr.indexCount) {
            fclose(f);
            throw std::runtime_error("SmeshLoader: index read error in " + path);
        }

        fclose(f);

        LOG_DEBUG("SmeshLoader: loaded " + path
            + " (" + std::to_string(hdr.vertexCount) + " verts, "
            + std::to_string(hdr.indexCount)  + " indices)");

        return renderer::Mesh(device, context, vertices,
            std::vector<unsigned int>(indices.begin(), indices.end()));
    }
}