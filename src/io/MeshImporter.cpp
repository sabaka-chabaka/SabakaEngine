#define NOMINMAX
#include "io/MeshImporter.h"
#include "io/SmeshFormat.h"
#include "renderer/Mesh.h"
#include "core/Logger.h"
#include "../third_party/tinyobjloader/tiny_obj_loader.h"
#include <DirectXMath.h>
#include <filesystem>
#include <map>
#include <tuple>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <limits>

namespace engine::io {

    using namespace DirectX;

    struct ObjIdx {
        int pos, uv, nor;
        bool operator<(const ObjIdx& o) const {
            return std::tie(pos, uv, nor) < std::tie(o.pos, o.uv, o.nor);
        }
    };

    static void computeTangents(
        std::vector<renderer::Vertex>&   verts,
        const std::vector<uint32_t>&     indices)
    {
        std::vector<XMFLOAT3> tan(verts.size(),   { 0.f, 0.f, 0.f });
        std::vector<XMFLOAT3> bitan(verts.size(), { 0.f, 0.f, 0.f });

        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
            auto& v0 = verts[indices[i]];
            auto& v1 = verts[indices[i + 1]];
            auto& v2 = verts[indices[i + 2]];

            XMFLOAT3 dP1 = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
            XMFLOAT3 dP2 = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };

            float dU1 = v1.u - v0.u, dV1 = v1.v - v0.v;
            float dU2 = v2.u - v0.u, dV2 = v2.v - v0.v;

            float det = dU1 * dV2 - dU2 * dV1;
            if (fabsf(det) < 1e-8f) continue;

            float r = 1.0f / det;
            XMFLOAT3 t = {
                (dP1.x * dV2 - dP2.x * dV1) * r,
                (dP1.y * dV2 - dP2.y * dV1) * r,
                (dP1.z * dV2 - dP2.z * dV1) * r,
            };
            XMFLOAT3 b = {
                (dP2.x * dU1 - dP1.x * dU2) * r,
                (dP2.y * dU1 - dP1.y * dU2) * r,
                (dP2.z * dU1 - dP1.z * dU2) * r,
            };

            for (int k = 0; k < 3; ++k) {
                uint32_t idx = indices[i + k];
                tan[idx].x   += t.x; tan[idx].y += t.y; tan[idx].z += t.z;
                bitan[idx].x += b.x; bitan[idx].y += b.y; bitan[idx].z += b.z;
            }
        }

        for (size_t i = 0; i < verts.size(); ++i) {
            auto& v = verts[i];
            XMVECTOR N = XMVectorSet(v.nx, v.ny, v.nz, 0.f);
            XMVECTOR T = XMVectorSet(tan[i].x, tan[i].y, tan[i].z, 0.f);
            XMVECTOR B = XMVectorSet(bitan[i].x, bitan[i].y, bitan[i].z, 0.f);

            T = XMVector3Normalize(T - N * XMVector3Dot(N, T));
            float hand = XMVectorGetX(XMVector3Dot(XMVector3Cross(N, T), B)) < 0.f ? -1.f : 1.f;

            v.tx = XMVectorGetX(T);
            v.ty = XMVectorGetY(T);
            v.tz = XMVectorGetZ(T);
            v.tw = hand;
        }
    }

    void MeshImporter::importObj(const std::string& objPath, const std::string& smeshPath)
    {
        LOG_INFO("MeshImporter: importing " + objPath + " → " + smeshPath);

        tinyobj::attrib_t                attrib;
        std::vector<tinyobj::shape_t>    shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objPath.c_str()))
            throw std::runtime_error("MeshImporter: tinyobj error: " + err + " (" + objPath + ")");

        if (!warn.empty()) LOG_WARN("MeshImporter: " + warn);

        std::map<ObjIdx, uint32_t>   indexMap;
        std::vector<renderer::Vertex> vertices;
        std::vector<uint32_t>         indices;

        for (auto& shape : shapes) {
            for (auto& idx : shape.mesh.indices) {
                ObjIdx key{ idx.vertex_index, idx.texcoord_index, idx.normal_index };
                auto it = indexMap.find(key);
                if (it != indexMap.end()) {
                    indices.push_back(it->second);
                    continue;
                }

                renderer::Vertex v = {};
                if (idx.vertex_index >= 0) {
                    int i = idx.vertex_index * 3;
                    v.x = attrib.vertices[i];
                    v.y = attrib.vertices[i + 1];
                    v.z = attrib.vertices[i + 2];
                }
                if (idx.texcoord_index >= 0) {
                    int i = idx.texcoord_index * 2;
                    v.u =        attrib.texcoords[i];
                    v.v = 1.f  - attrib.texcoords[i + 1];
                }
                if (idx.normal_index >= 0) {
                    int i = idx.normal_index * 3;
                    v.nx = attrib.normals[i];
                    v.ny = attrib.normals[i + 1];
                    v.nz = attrib.normals[i + 2];
                }
                v.r = 1.f; v.g = 1.f; v.b = 1.f;

                uint32_t newIdx   = static_cast<uint32_t>(vertices.size());
                indexMap[key]     = newIdx;
                vertices.push_back(v);
                indices.push_back(newIdx);
            }
        }

        if (vertices.empty())
            throw std::runtime_error("MeshImporter: no geometry in " + objPath);

        computeTangents(vertices, indices);

        constexpr float kMax = (std::numeric_limits<float>::max)();
        float minX =  kMax, minY =  kMax, minZ =  kMax;
        float maxX = -kMax, maxY = -kMax, maxZ = -kMax;

        for (auto& v : vertices) {
            minX = (std::min)(minX, v.x); maxX = (std::max)(maxX, v.x);
            minY = (std::min)(minY, v.y); maxY = (std::max)(maxY, v.y);
            minZ = (std::min)(minZ, v.z); maxZ = (std::max)(maxZ, v.z);
        }

        SmeshHeader hdr{};
        memcpy(hdr.magic,  SMESH_MAGIC, 8);
        hdr.version     = SMESH_VERSION;
        hdr.vertexCount = static_cast<uint32_t>(vertices.size());
        hdr.indexCount  = static_cast<uint32_t>(indices.size());
        hdr.aabbMin[0]  = minX; hdr.aabbMin[1] = minY; hdr.aabbMin[2] = minZ;
        hdr.aabbMax[0]  = maxX; hdr.aabbMax[1] = maxY; hdr.aabbMax[2] = maxZ;

        std::filesystem::create_directories(
            std::filesystem::path(smeshPath).parent_path());

        FILE* f = fopen(smeshPath.c_str(), "wb");
        if (!f)
            throw std::runtime_error("MeshImporter: cannot write " + smeshPath);

        fwrite(&hdr,           sizeof(hdr),                  1,                  f);
        fwrite(vertices.data(), sizeof(renderer::Vertex),    vertices.size(),    f);
        fwrite(indices.data(),  sizeof(uint32_t),            indices.size(),     f);
        fclose(f);

        LOG_INFO("MeshImporter: wrote " + smeshPath
            + " (" + std::to_string(hdr.vertexCount) + " verts, "
            + std::to_string(hdr.indexCount) + " indices)");
    }

    std::string MeshImporter::smeshPathFor(const std::string& srcPath)
    {
        std::filesystem::path p(srcPath);
        return (p.parent_path() / p.stem()).string() + ".smesh";
    }

    bool MeshImporter::needsReimport(const std::string& srcPath, const std::string& smeshPath)
    {
        namespace fs = std::filesystem;

        if (!fs::exists(smeshPath)) return true;

        std::error_code ec;
        auto srcTime   = fs::last_write_time(srcPath,   ec); if (ec) return true;
        auto cacheTime = fs::last_write_time(smeshPath, ec); if (ec) return true;

        return srcTime > cacheTime;
    }

}