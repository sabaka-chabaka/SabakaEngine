#pragma once
#include "Mesh.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

namespace engine::renderer {
    using namespace DirectX;
    using Microsoft::WRL::ComPtr;

    struct InstanceData {
        XMFLOAT4X4 worldMatrix;
    };

    class InstancedMesh {
    public:
        InstancedMesh(
            ID3D11Device*        device,
            ID3D11DeviceContext* context,
            Mesh*                mesh,
            uint32_t             maxInstances
        );

        ~InstancedMesh() = default;

        InstancedMesh(const InstancedMesh&)            = delete;
        InstancedMesh& operator=(const InstancedMesh&) = delete;

        void setInstances(const std::vector<XMMATRIX>& worldMatrices);
        void draw() const;

        uint32_t getInstanceCount() const;

    private:
        ID3D11DeviceContext*  m_context;
        Mesh*                 m_mesh;
        uint32_t              m_maxInstances;
        uint32_t              m_instanceCount = 0;

        ComPtr<ID3D11Buffer>  m_instanceBuffer;
    };
}