#pragma once
#include <d3d11.h>
#include <wrl/client.h>

namespace engine::renderer {
    using Microsoft::WRL::ComPtr;

    class SkyboxMesh {
    public:
        SkyboxMesh(ID3D11Device* device, ID3D11DeviceContext* context);

        ~SkyboxMesh() = default;

        SkyboxMesh(const SkyboxMesh&)            = delete;
        SkyboxMesh& operator=(const SkyboxMesh&) = delete;

        void draw() const;

    private:
        ID3D11DeviceContext* m_context      = nullptr;
        unsigned int         m_indexCount   = 0;
        unsigned int         m_vertexStride = 0;
        unsigned int         m_vertexOffset = 0;

        ComPtr<ID3D11Buffer> m_vertexBuffer;
        ComPtr<ID3D11Buffer> m_indexBuffer;
    };
}