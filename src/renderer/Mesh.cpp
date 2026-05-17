#include "renderer/Mesh.h"
#include <stdexcept>
#include "core/Logger.h"

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

    Mesh Mesh::createCube(ID3D11Device *device, ID3D11DeviceContext *context) {
        std::vector<Vertex> vertices = {
            {-0.5f, 0.5f, -0.5f, 1.0f, 0.2f, 0.2f},
            {0.5f, 0.5f, -0.5f, 1.0f, 0.2f, 0.2f},
            {-0.5f, -0.5f, -0.5f, 1.0f, 0.2f, 0.2f},
            {0.5f, -0.5f, -0.5f, 1.0f, 0.2f, 0.2f},

            {0.5f, 0.5f, 0.5f, 0.2f, 1.0f, 0.2f},
            {-0.5f, 0.5f, 0.5f, 0.2f, 1.0f, 0.2f},
            {0.5f, -0.5f, 0.5f, 0.2f, 1.0f, 0.2f},
            {-0.5f, -0.5f, 0.5f, 0.2f, 1.0f, 0.2f},

            {-0.5f, 0.5f, 0.5f, 0.2f, 0.2f, 1.0f},
            {-0.5f, 0.5f, -0.5f, 0.2f, 0.2f, 1.0f},
            {-0.5f, -0.5f, 0.5f, 0.2f, 0.2f, 1.0f},
            {-0.5f, -0.5f, -0.5f, 0.2f, 0.2f, 1.0f},

            {0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.2f},
            {0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.2f},
            {0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.2f},
            {0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.2f},

            {-0.5f, 0.5f, 0.5f, 0.2f, 1.0f, 1.0f},
            {0.5f, 0.5f, 0.5f, 0.2f, 1.0f, 1.0f},
            {-0.5f, 0.5f, -0.5f, 0.2f, 1.0f, 1.0f},
            {0.5f, 0.5f, -0.5f, 0.2f, 1.0f, 1.0f},

            {-0.5f, -0.5f, -0.5f, 1.0f, 0.2f, 1.0f},
            {0.5f, -0.5f, -0.5f, 1.0f, 0.2f, 1.0f},
            {-0.5f, -0.5f, 0.5f, 1.0f, 0.2f, 1.0f},
            {0.5f, -0.5f, 0.5f, 1.0f, 0.2f, 1.0f},
        };

        std::vector<unsigned int> indices = {
            0, 1, 2, 1, 3, 2,
            4, 5, 6, 5, 7, 6,
            8, 9, 10, 9, 11, 10,
            12, 13, 14, 13, 15, 14,
            16, 17, 18, 17, 19, 18,
            20, 21, 22, 21, 23, 22,
        };

        return Mesh(device, context, vertices, indices);
    }
}