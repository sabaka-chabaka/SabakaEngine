#include "SkyboxMesh.h"
#include <stdexcept>
#include <array>

namespace engine::renderer {
    struct SkyboxVertex {
        float x, y, z;
    };

    SkyboxMesh::SkyboxMesh(ID3D11Device *device, ID3D11DeviceContext *context)
        : m_context(context)
          , m_vertexStride(sizeof(SkyboxVertex))
          , m_vertexOffset(0) {
        static const std::array<SkyboxVertex, 8> vertices = {
            {
                {-1.0f, -1.0f, -1.0f},
                {1.0f, -1.0f, -1.0f},
                {1.0f, 1.0f, -1.0f},
                {-1.0f, 1.0f, -1.0f},
                {-1.0f, -1.0f, 1.0f},
                {1.0f, -1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f},
                {-1.0f, 1.0f, 1.0f},
            }
        };

        static const std::array<unsigned int, 36> indices = {
            {
                0, 1, 2, 0, 2, 3,
                5, 4, 7, 5, 7, 6,
                4, 0, 3, 4, 3, 7,
                1, 5, 6, 1, 6, 2,
                3, 2, 6, 3, 6, 7,
                4, 5, 1, 4, 1, 0,
            }
        };

        m_indexCount = static_cast<unsigned int>(indices.size());

        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(vertices);
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA data = {};
            data.pSysMem = vertices.data();

            if (FAILED(device->CreateBuffer(&desc, &data, &m_vertexBuffer))) {
                throw std::runtime_error("Failed to create skybox vertex buffer");
            }
        }

        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(indices);
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA data = {};
            data.pSysMem = indices.data();

            if (FAILED(device->CreateBuffer(&desc, &data, &m_indexBuffer))) {
                throw std::runtime_error("Failed to create skybox index buffer");
            }
        }
    }

    void SkyboxMesh::draw() const {
        m_context->IASetVertexBuffers(
            0, 1,
            m_vertexBuffer.GetAddressOf(),
            &m_vertexStride,
            &m_vertexOffset
        );
        m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->DrawIndexed(m_indexCount, 0, 0);
    }
}
