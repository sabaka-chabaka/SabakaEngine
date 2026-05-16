#include "GraphicsDevice.h"

#include <stdexcept>

namespace engine::renderer {
    GraphicsDevice::GraphicsDevice(const GraphicsDeviceDesc& desc)
    : m_vsync(desc.vsync), m_width(desc.width), m_height(desc.height)
    {
        DXGI_SWAP_CHAIN_DESC scd               = {};
        scd.BufferCount                        = 2;
        scd.BufferDesc.Width                   = m_width;
        scd.BufferDesc.Height                  = m_height;
        scd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.RefreshRate.Numerator   = 60;
        scd.BufferDesc.RefreshRate.Denominator = 1;
        scd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow                       = desc.hwnd;
        scd.SampleDesc.Count                   = 1;
        scd.Windowed                           = TRUE;
        scd.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        D3D_FEATURE_LEVEL featureLevel;
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

        UINT flags = 0;
#ifdef _DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &scd,
            &m_swapChain,
            &m_device,
            &featureLevel,
            &m_context
        );

        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create D3D11 device and swap chain");
        }

        createRenderTargetView();
    }

    void GraphicsDevice::createRenderTargetView() {
        ComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to get back buffer");
        }

        hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create render target view");
        }

        D3D11_VIEWPORT viewport = {};
        viewport.Width    = static_cast<float>(m_width);
        viewport.Height   = static_cast<float>(m_height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        m_context->RSSetViewports(1, &viewport);
        m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
    }

    void GraphicsDevice::releaseRenderTargetView() {
        m_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_renderTargetView.Reset();
    }

    void GraphicsDevice::beginFrame(float r, float g, float b) {
        m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

        float color[4] = { r, g, b, 1.0f };
        m_context->ClearRenderTargetView(m_renderTargetView.Get(), color);
    }

    void GraphicsDevice::endFrame() {
        m_swapChain->Present(m_vsync ? 1 : 0, 0);
    }

    void GraphicsDevice::onResize(int width, int height) {
        if (width == 0 || height == 0) return;

        m_width  = width;
        m_height = height;

        releaseRenderTargetView();

        HRESULT hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to resize swap chain buffers");
        }

        createRenderTargetView();
    }

    ID3D11Device*        GraphicsDevice::getDevice()       const { return m_device.Get(); }
    ID3D11DeviceContext* GraphicsDevice::getDeviceContext() const { return m_context.Get(); }

}