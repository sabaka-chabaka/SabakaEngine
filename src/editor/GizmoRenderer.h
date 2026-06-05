#pragma once
#define NOMINMAX
#include "editor/GizmoMode.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

namespace engine::renderer {
    class GraphicsDevice;
    class Camera;
}

namespace engine::editor {

    class GizmoRenderer {
    public:
        GizmoRenderer() = default;

        void init(renderer::GraphicsDevice* device);
        void render(renderer::GraphicsDevice* device,
                    renderer::Camera*          camera,
                    const DirectX::XMFLOAT3&   origin,
                    GizmoMode                  mode,
                    GizmoAxis                  hovered);

        bool isInitialized() const { return m_initialized; }

    private:
        struct GizmoVertex {
            DirectX::XMFLOAT3 pos;
            DirectX::XMFLOAT3 color;
        };

        void buildArrowMesh(std::vector<GizmoVertex>& verts,
                            std::vector<uint32_t>&    inds,
                            const DirectX::XMFLOAT3&  dir,
                            const DirectX::XMFLOAT3&  col,
                            uint32_t                  baseVert);

        void buildCircleMesh(std::vector<GizmoVertex>& verts,
                             std::vector<uint32_t>&    inds,
                             const DirectX::XMFLOAT3&  normal,
                             const DirectX::XMFLOAT3&  col,
                             uint32_t                  baseVert,
                             int                       segments = 48);

        void buildBoxHandle(std::vector<GizmoVertex>& verts,
                            std::vector<uint32_t>&    inds,
                            const DirectX::XMFLOAT3&  dir,
                            const DirectX::XMFLOAT3&  col,
                            uint32_t                  baseVert);

        void uploadAndDraw(renderer::GraphicsDevice*       device,
                           const std::vector<GizmoVertex>& verts,
                           const std::vector<uint32_t>&    inds);

        void setTransformCB(renderer::GraphicsDevice*  device,
                            renderer::Camera*           camera,
                            const DirectX::XMFLOAT3&   origin,
                            float                       scale);

        bool compileShaders(ID3D11Device* device);

        Microsoft::WRL::ComPtr<ID3D11VertexShader>  m_vs;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>   m_ps;
        Microsoft::WRL::ComPtr<ID3D11InputLayout>   m_layout;
        Microsoft::WRL::ComPtr<ID3D11Buffer>        m_cbTransform;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizer;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthState;

        bool m_initialized = false;
    };

}
