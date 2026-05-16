#include "Mesh.h"

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

    unsigned int Mesh::getIndexCount() const {
        return m_indexCount;
    }
}