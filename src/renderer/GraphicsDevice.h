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

    class GraphicsDevice {
    public:
        explicit GraphicsDevice(const GraphicsDeviceDesc& desc);
        ~GraphicsDevice() = default;

        GraphicsDevice(const GraphicsDevice&)            = delete;
        GraphicsDevice& operator=(const GraphicsDevice&) = delete;

        void beginFrame(float r, float g, float b);
        void endFrame();

        void onResize(int width, int height);

        ID3D11Device*        getDevice()        const;
        ID3D11DeviceContext* getDeviceContext()  const;

    private:
        void createRenderTargetView();
        void releaseRenderTargetView();

        ComPtr<ID3D11Device>           m_device;
        ComPtr<ID3D11DeviceContext>    m_context;
        ComPtr<IDXGISwapChain>         m_swapChain;
        ComPtr<ID3D11RenderTargetView> m_renderTargetView;

        bool m_vsync  = true;
        int  m_width  = 0;
        int  m_height = 0;
    };
}