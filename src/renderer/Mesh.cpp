#include "renderer/Mesh.h"
#include "core/Logger.h"
#include <stdexcept>
#include <array>

namespace engine::renderer {
    Mesh::Mesh(ID3D11Device *device,
               ID3D11DeviceContext *context,
               const std::vector<Vertex> &vertices,
               const std::vector<unsigned int> &indices)
        : m_context(context)
          , m_indexCount(static_cast<unsigned int>(indices.size()))
          , m_vertexStride(sizeof(Vertex))
          , m_vertexOffset(0) {
        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            desc.CPUAccessFlags = 0;

            D3D11_SUBRESOURCE_DATA data = {};
            data.pSysMem = vertices.data();

            HRESULT hr = device->CreateBuffer(&desc, &data, &m_vertexBuffer);
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to create vertex buffer");
            }

            LOG_DEBUG("Vertex buffer created: " + std::to_string(vertices.size()) + " vertices");
        }

        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = static_cast<UINT>(sizeof(unsigned int) * indices.size());
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            desc.CPUAccessFlags = 0;

            D3D11_SUBRESOURCE_DATA data = {};
            data.pSysMem = indices.data();

            HRESULT hr = device->CreateBuffer(&desc, &data, &m_indexBuffer);
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to create index buffer");
            }

            LOG_DEBUG("Index buffer created: " + std::to_string(indices.size()) + " indices");
        }

        LOG_DEBUG("Mesh created: " +
            std::to_string(vertices.size()) + " verts, " +
            std::to_string(indices.size()) + " indices");
    }

    void Mesh::draw() const {
        m_context->IASetVertexBuffers(
            0, 1,
            m_vertexBuffer.GetAddressOf(),
            &m_vertexStride,
            &m_vertexOffset
        );

        m_context->IASetIndexBuffer(
            m_indexBuffer.Get(),
            DXGI_FORMAT_R32_UINT,
            0
        );

        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        m_context->DrawIndexed(m_indexCount, 0, 0);
    }

    void Mesh::bindBuffers() const {
        m_context->IASetVertexBuffers(
            0, 1,
            m_vertexBuffer.GetAddressOf(),
            &m_vertexStride,
            &m_vertexOffset
        );

        m_context->IASetIndexBuffer(
            m_indexBuffer.Get(),
            DXGI_FORMAT_R32_UINT,
            0
        );

        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void Mesh::drawInstanced(uint32_t instanceCount) const {
        m_context->DrawIndexedInstanced(m_indexCount, instanceCount, 0, 0, 0);
    }

    static std::array<float, 4> calcFaceTangent(
        const Vertex& v0, const Vertex& v1, const Vertex& v2)
    {
        float e1x = v1.x - v0.x, e1y = v1.y - v0.y, e1z = v1.z - v0.z;
        float e2x = v2.x - v0.x, e2y = v2.y - v0.y, e2z = v2.z - v0.z;

        float du1 = v1.u - v0.u, dv1 = v1.v - v0.v;
        float du2 = v2.u - v0.u, dv2 = v2.v - v0.v;

        float det = du1 * dv2 - du2 * dv1;
        if (fabsf(det) < 1e-6f) return { 1.0f, 0.0f, 0.0f, 1.0f };

        float r = 1.0f / det;
        float tx = r * (dv2 * e1x - dv1 * e2x);
        float ty = r * (dv2 * e1y - dv1 * e2y);
        float tz = r * (dv2 * e1z - dv1 * e2z);

        float bx = r * (du1 * e2x - du2 * e1x);
        float by = r * (du1 * e2y - du2 * e1y);
        float bz = r * (du1 * e2z - du2 * e1z);

        float len = sqrtf(tx*tx + ty*ty + tz*tz);
        if (len < 1e-6f) return { 1.0f, 0.0f, 0.0f, 1.0f };
        
        tx /= len; ty /= len; tz /= len;

        // B = cross(N, T) * handedness
        // handedness = dot(cross(N, T), B)
        float nx = v0.nx, ny = v0.ny, nz = v0.nz;
        float cx = ny * tz - nz * ty;
        float cy = nz * tx - nx * tz;
        float cz = nx * ty - ny * tx;

        float h = (cx * bx + cy * by + cz * bz) < 0.0f ? -1.0f : 1.0f;

        return { tx, ty, tz, h };
    }

    Mesh Mesh::createCube(ID3D11Device *device, ID3D11DeviceContext *context) {
        std::vector<Vertex> vertices = {
            {-0.5f,  0.5f, -0.5f,  1.0f,0.2f,0.2f,  0.0f,0.0f,  0.0f,0.0f,-1.0f,  0,0,0,0},
            { 0.5f,  0.5f, -0.5f,  1.0f,0.2f,0.2f,  1.0f,0.0f,  0.0f,0.0f,-1.0f,  0,0,0,0},
            {-0.5f, -0.5f, -0.5f,  1.0f,0.2f,0.2f,  0.0f,1.0f,  0.0f,0.0f,-1.0f,  0,0,0,0},
            { 0.5f, -0.5f, -0.5f,  1.0f,0.2f,0.2f,  1.0f,1.0f,  0.0f,0.0f,-1.0f,  0,0,0,0},

            { 0.5f,  0.5f,  0.5f,  0.2f,1.0f,0.2f,  0.0f,0.0f,  0.0f,0.0f,1.0f,  0,0,0,0},
            {-0.5f,  0.5f,  0.5f,  0.2f,1.0f,0.2f,  1.0f,0.0f,  0.0f,0.0f,1.0f,  0,0,0,0},
            { 0.5f, -0.5f,  0.5f,  0.2f,1.0f,0.2f,  0.0f,1.0f,  0.0f,0.0f,1.0f,  0,0,0,0},
            {-0.5f, -0.5f,  0.5f,  0.2f,1.0f,0.2f,  1.0f,1.0f,  0.0f,0.0f,1.0f,  0,0,0,0},

            {-0.5f,  0.5f,  0.5f,  0.2f,0.2f,1.0f,  0.0f,0.0f,  -1.0f,0.0f,0.0f,  0,0,0,0},
            {-0.5f,  0.5f, -0.5f,  0.2f,0.2f,1.0f,  1.0f,0.0f,  -1.0f,0.0f,0.0f,  0,0,0,0},
            {-0.5f, -0.5f,  0.5f,  0.2f,0.2f,1.0f,  0.0f,1.0f,  -1.0f,0.0f,0.0f,  0,0,0,0},
            {-0.5f, -0.5f, -0.5f,  0.2f,0.2f,1.0f,  1.0f,1.0f,  -1.0f,0.0f,0.0f,  0,0,0,0},

            { 0.5f,  0.5f, -0.5f,  1.0f,1.0f,0.2f,  0.0f,0.0f,  1.0f,0.0f,0.0f,  0,0,0,0},
            { 0.5f,  0.5f,  0.5f,  1.0f,1.0f,0.2f,  1.0f,0.0f,  1.0f,0.0f,0.0f,  0,0,0,0},
            { 0.5f, -0.5f, -0.5f,  1.0f,1.0f,0.2f,  0.0f,1.0f,  1.0f,0.0f,0.0f,  0,0,0,0},
            { 0.5f, -0.5f,  0.5f,  1.0f,1.0f,0.2f,  1.0f,1.0f,  1.0f,0.0f,0.0f,  0,0,0,0},

            {-0.5f,  0.5f,  0.5f,  0.2f,1.0f,1.0f,  0.0f,0.0f,  0.0f,1.0f,0.0f,  0,0,0,0},
            { 0.5f,  0.5f,  0.5f,  0.2f,1.0f,1.0f,  1.0f,0.0f,  0.0f,1.0f,0.0f,  0,0,0,0},
            {-0.5f,  0.5f, -0.5f,  0.2f,1.0f,1.0f,  0.0f,1.0f,  0.0f,1.0f,0.0f,  0,0,0,0},
            { 0.5f,  0.5f, -0.5f,  0.2f,1.0f,1.0f,  1.0f,1.0f,  0.0f,1.0f,0.0f,  0,0,0,0},

            {-0.5f, -0.5f, -0.5f,  1.0f,0.2f,1.0f,  0.0f,0.0f,  0.0f,-1.0f,0.0f,  0,0,0,0},
            { 0.5f, -0.5f, -0.5f,  1.0f,0.2f,1.0f,  1.0f,0.0f,  0.0f,-1.0f,0.0f,  0,0,0,0},
            {-0.5f, -0.5f,  0.5f,  1.0f,0.2f,1.0f,  0.0f,1.0f,  0.0f,-1.0f,0.0f,  0,0,0,0},
            { 0.5f, -0.5f,  0.5f,  1.0f,0.2f,1.0f,  1.0f,1.0f,  0.0f,-1.0f,0.0f,  0,0,0,0},
        };

        for (int face = 0; face < 6; ++face) {
            int base = face * 4;
            auto t = calcFaceTangent(vertices[base], vertices[base+1], vertices[base+2]);
            for (int i = 0; i < 4; ++i) {
                vertices[base + i].tx = t[0];
                vertices[base + i].ty = t[1];
                vertices[base + i].tz = t[2];
                vertices[base + i].tw = t[3];
            }
        }

        std::vector<unsigned int> indices = {
            0,  1,  2,   1,  3,  2,
            4,  5,  6,   5,  7,  6,
            8,  9,  10,  9,  11, 10,
            12, 13, 14,  13, 15, 14,
            16, 17, 18,  17, 19, 18,
            20, 21, 22,  21, 23, 22,
        };

        return Mesh(device, context, vertices, indices);
    }
}