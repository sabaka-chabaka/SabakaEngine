#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

namespace engine::renderer {
    using Microsoft::WRL::ComPtr;

    struct Vertex {
        float x, y, z;
        float r, g, b;
    };

    class Mesh {
    public:
        Mesh(ID3D11Device *device,
             ID3D11DeviceContext *context,
             const std::vector<Vertex> &vertices,
             const std::vector<unsigned int> &indices);

        ~Mesh() = default;

        Mesh(const Mesh&)            = delete;
        Mesh& operator=(const Mesh&) = delete;

        Mesh(Mesh&&)                 = default;
        Mesh& operator=(Mesh&&)      = default;

        void draw() const;

        static Mesh createCube(ID3D11Device* device, ID3D11DeviceContext* context);

    private:
        ID3D11DeviceContext*          m_context;

        ComPtr<ID3D11Buffer>          m_vertexBuffer;
        ComPtr<ID3D11Buffer>          m_indexBuffer;

        unsigned int                  m_indexCount  = 0;
        unsigned int                  m_vertexStride = 0;
        unsigned int                  m_vertexOffset = 0;
    };
}