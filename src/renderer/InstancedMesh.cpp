#include "renderer/InstancedMesh.h"
#include <stdexcept>

namespace engine::renderer {

    InstancedMesh::InstancedMesh(
        ID3D11Device*        device,
        ID3D11DeviceContext* context,
        Mesh*                mesh,
        uint32_t             maxInstances
    )
        : m_context(context)
        , m_mesh(mesh)
        , m_maxInstances(maxInstances)
    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth      = sizeof(InstanceData) * maxInstances;
        desc.Usage          = D3D11_USAGE_DYNAMIC;
        desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        if (FAILED(device->CreateBuffer(&desc, nullptr, &m_instanceBuffer)))
            throw std::runtime_error("InstancedMesh: failed to create instance buffer");
    }

    void InstancedMesh::setInstances(const std::vector<XMMATRIX>& worldMatrices) {
        m_instanceCount = static_cast<uint32_t>(
            min((size_t)m_maxInstances, worldMatrices.size())
        );

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (FAILED(m_context->Map(m_instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            return;

        auto* dst = static_cast<InstanceData*>(mapped.pData);
        for (uint32_t i = 0; i < m_instanceCount; ++i) {
            XMStoreFloat4x4(&dst[i].worldMatrix, XMMatrixTranspose(worldMatrices[i]));
        }

        m_context->Unmap(m_instanceBuffer.Get(), 0);
    }

    void InstancedMesh::draw() const {
        if (m_instanceCount == 0) return;

        m_mesh->bindBuffers();

        ID3D11Buffer* instanceBuf = m_instanceBuffer.Get();
        UINT          stride      = sizeof(InstanceData);
        UINT          offset      = 0;
        m_context->IASetVertexBuffers(1, 1, &instanceBuf, &stride, &offset);

        m_mesh->drawInstanced(m_instanceCount);
    }

    uint32_t InstancedMesh::getInstanceCount() const {
        return m_instanceCount;
    }
}