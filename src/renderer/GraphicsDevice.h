#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <windows.h>

namespace engine::renderer {
    using Microsoft::WRL::ComPtr;

    struct GraphicsDeviceDesc {
        HWND hwnd = nullptr;
        int width = 1280;
        int height = 720;
        bool vsync = true;
    };

    enum class FillMode { Solid, Wireframe };

    enum class CullMode { None, Front, Back };

    enum class BlendMode { Opaque, AlphaBlend, Additive, Multiplicative };

    enum class DepthFunc { Less, LessEqual };

    class GraphicsDevice {
    public:
        explicit GraphicsDevice(const GraphicsDeviceDesc &desc);

        ~GraphicsDevice() = default;

        GraphicsDevice(const GraphicsDevice &) = delete;

        GraphicsDevice &operator=(const GraphicsDevice &) = delete;

        void beginFrame(float r, float g, float b);

        void endFrame();

        void onResize(int width, int height);

        void setFillMode(FillMode mode);

        void setCullMode(CullMode mode);

        void setBlendMode(BlendMode mode);

        void setDepthWriteEnabled(bool enabled);

        void setDepthFunc(DepthFunc func);

        ID3D11Device *getDevice() const;

        ID3D11DeviceContext *getDeviceContext() const;

    private:
        void createRenderTargetView();

        void releaseRenderTargetView();

        void createDepthStencilBuffer(int width, int height);

        void releaseDepthStencilBuffer();

        void rebuildRasterizerState();

        void rebuildBlendState();

        void rebuildDepthStencilState();

        ComPtr<ID3D11Device> m_device;
        ComPtr<ID3D11DeviceContext> m_context;
        ComPtr<IDXGISwapChain> m_swapChain;
        ComPtr<ID3D11RenderTargetView> m_renderTargetView;
        ComPtr<ID3D11Texture2D> m_depthStencilTexture;
        ComPtr<ID3D11DepthStencilView> m_depthStencilView;
        ComPtr<ID3D11DepthStencilState> m_depthStencilState;
        ComPtr<ID3D11RasterizerState> m_rasterizerState;
        ComPtr<ID3D11BlendState> m_blendState;

        FillMode m_fillMode = FillMode::Solid;
        CullMode m_cullMode = CullMode::Back;
        BlendMode m_blendMode = BlendMode::Opaque;
        DepthFunc m_depthFunc = DepthFunc::Less;
        bool m_depthWrite = true;
        bool m_vsync = true;
        int m_width = 0;
        int m_height = 0;
    };
}