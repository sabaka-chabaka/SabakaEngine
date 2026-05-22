#define TINYOBJLOADER_IMPLEMENTATION
#include "../third_party/tinyobjloader/tiny_obj_loader.h"
#include "ObjLoader.h"
#include <DirectXMath.h>
#include <map>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace engine::io {
    using namespace DirectX;

    struct ObjIndex {
        int pos, uv, nor;
        bool operator<(const ObjIndex& other) const {
            return std::tie(pos, uv, nor) < std::tie(other.pos, other.uv, other.nor);
        }
    };

    static void computeTangents(
        std::vector<renderer::Vertex>&   verts,
        const std::vector<unsigned int>& indices
    ) {
        std::vector<XMFLOAT3> tan(verts.size(), { 0.f, 0.f, 0.f });

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

            for (int k = 0; k < 3; ++k) {
                unsigned int idx = indices[i + k];
                tan[idx].x += t.x;
                tan[idx].y += t.y;
                tan[idx].z += t.z;
            }
        }

        for (size_t i = 0; i < verts.size(); ++i) {
            auto& v = verts[i];
            XMVECTOR N = XMVectorSet(v.nx, v.ny, v.nz, 0.f);
            XMVECTOR T = XMVectorSet(tan[i].x, tan[i].y, tan[i].z, 0.f);
            T = XMVector3Normalize(T - N * XMVector3Dot(N, T));
            v.tx = XMVectorGetX(T);
            v.ty = XMVectorGetY(T);
            v.tz = XMVectorGetZ(T);
        }
    }

    renderer::Mesh ObjLoader::load(
        ID3D11Device*        device,
        ID3D11DeviceContext* context,
        const std::string&   path
    ) {
        tinyobj::attrib_t                attrib;
        std::vector<tinyobj::shape_t>    shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
        if (!ok)
            throw std::runtime_error("ObjLoader: " + err + " (" + path + ")");

        std::map<ObjIndex, unsigned int> indexMap;
        std::vector<renderer::Vertex>    vertices;
        std::vector<unsigned int>        indices;

        for (auto& shape : shapes) {
            for (auto& idx : shape.mesh.indices) {
                ObjIndex key { idx.vertex_index, idx.texcoord_index, idx.normal_index };

                auto it = indexMap.find(key);
                if (it != indexMap.end()) {
                    indices.push_back(it->second);
                    continue;
                }

                renderer::Vertex v = {};

                if (idx.vertex_index >= 0) {
                    int i  = idx.vertex_index * 3;
                    v.x    = attrib.vertices[i];
                    v.y    = attrib.vertices[i + 1];
                    v.z    = attrib.vertices[i + 2];
                }

                if (idx.texcoord_index >= 0) {
                    int i  = idx.texcoord_index * 2;
                    v.u    =       attrib.texcoords[i];
                    v.v    = 1.0f - attrib.texcoords[i + 1];
                }

                if (idx.normal_index >= 0) {
                    int i  = idx.normal_index * 3;
                    v.nx   = attrib.normals[i];
                    v.ny   = attrib.normals[i + 1];
                    v.nz   = attrib.normals[i + 2];
                }

                v.r = 1.0f; v.g = 1.0f; v.b = 1.0f;

                unsigned int newIdx = static_cast<unsigned int>(vertices.size());
                indexMap[key]       = newIdx;
                vertices.push_back(v);
                indices.push_back(newIdx);
            }
        }

        if (vertices.empty())
            throw std::runtime_error("ObjLoader: no geometry in: " + path);

        computeTangents(vertices, indices);

        return renderer::Mesh(device, context, vertices, indices);
    }
}