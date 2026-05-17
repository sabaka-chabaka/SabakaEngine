#include "renderer/GraphicsDevice.h"
#include "core/Logger.h"
#include <stdexcept>

namespace engine::renderer {
    GraphicsDevice::GraphicsDevice(const GraphicsDeviceDesc &desc)
        : m_vsync(desc.vsync), m_width(desc.width), m_height(desc.height) {
        DXGI_SWAP_CHAIN_DESC scd = {};
        scd.BufferCount = 2;
        scd.BufferDesc.Width = m_width;
        scd.BufferDesc.Height = m_height;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.RefreshRate.Numerator = 60;
        scd.BufferDesc.RefreshRate.Denominator = 1;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = desc.hwnd;
        scd.SampleDesc.Count = 1;
        scd.Windowed = TRUE;
        scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
        D3D_FEATURE_LEVEL featureLevel;

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
        createDepthStencilBuffer(m_width, m_height);
        rebuildRasterizerState();

        LOG_INFO("GraphicsDevice ready");
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
    }

    void GraphicsDevice::releaseRenderTargetView() {
        m_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_renderTargetView.Reset();
    }

    void GraphicsDevice::createDepthStencilBuffer(int width, int height) {
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        HRESULT hr = m_device->CreateTexture2D(&texDesc, nullptr, &m_depthStencilTexture);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create depth stencil texture");
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        hr = m_device->CreateDepthStencilView(
            m_depthStencilTexture.Get(),
            &dsvDesc,
            &m_depthStencilView
        );
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create depth stencil view");
        }

        D3D11_DEPTH_STENCIL_DESC dsDesc = {};
        dsDesc.DepthEnable = TRUE;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
        dsDesc.StencilEnable = FALSE;

        hr = m_device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create depth stencil state");
        }

        LOG_DEBUG("Depth stencil buffer created (" +
            std::to_string(width) + "x" + std::to_string(height) + ")");
    }

    void GraphicsDevice::releaseDepthStencilBuffer() {
        m_depthStencilView.Reset();
        m_depthStencilTexture.Reset();
    }

    void GraphicsDevice::rebuildRasterizerState() {
        D3D11_RASTERIZER_DESC rsDesc = {};

        switch (m_fillMode) {
            case FillMode::Solid: rsDesc.FillMode = D3D11_FILL_SOLID;
                break;
            case FillMode::Wireframe: rsDesc.FillMode = D3D11_FILL_WIREFRAME;
                break;
        }

        switch (m_cullMode) {
            case CullMode::None: rsDesc.CullMode = D3D11_CULL_NONE;
                break;
            case CullMode::Front: rsDesc.CullMode = D3D11_CULL_FRONT;
                break;
            case CullMode::Back: rsDesc.CullMode = D3D11_CULL_BACK;
                break;
        }

        rsDesc.FrontCounterClockwise = FALSE;
        rsDesc.DepthClipEnable = TRUE;
        rsDesc.ScissorEnable = FALSE;
        rsDesc.MultisampleEnable = FALSE;
        rsDesc.AntialiasedLineEnable = FALSE;
        rsDesc.DepthBias = 0;
        rsDesc.DepthBiasClamp = 0.0f;
        rsDesc.SlopeScaledDepthBias = 0.0f;

        m_rasterizerState.Reset();
        HRESULT hr = m_device->CreateRasterizerState(&rsDesc, &m_rasterizerState);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create rasterizer state");
        }
    }

    void GraphicsDevice::beginFrame(float r, float g, float b) {
        m_context->OMSetRenderTargets(
            1,
            m_renderTargetView.GetAddressOf(),
            m_depthStencilView.Get()
        );

        m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
        m_context->RSSetState(m_rasterizerState.Get());

        float color[4] = {r, g, b, 1.0f};
        m_context->ClearRenderTargetView(m_renderTargetView.Get(), color);
        m_context->ClearDepthStencilView(
            m_depthStencilView.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f,
            0
        );

        D3D11_VIEWPORT viewport = {};
        viewport.Width = static_cast<float>(m_width);
        viewport.Height = static_cast<float>(m_height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        m_context->RSSetViewports(1, &viewport);
    }

    void GraphicsDevice::endFrame() {
        m_swapChain->Present(m_vsync ? 1 : 0, 0);
    }

    void GraphicsDevice::onResize(int width, int height) {
        if (width == 0 || height == 0) return;

        m_width = width;
        m_height = height;

        releaseRenderTargetView();
        releaseDepthStencilBuffer();

        HRESULT hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to resize swap chain buffers");
        }

        createRenderTargetView();
        createDepthStencilBuffer(width, height);

        LOG_DEBUG("Swap chain resized to " +
            std::to_string(width) + "x" + std::to_string(height));
    }

    void GraphicsDevice::setFillMode(FillMode mode) {
        m_fillMode = mode;
        rebuildRasterizerState();
    }

    void GraphicsDevice::setCullMode(CullMode mode) {
        m_cullMode = mode;
        rebuildRasterizerState();
    }

    ID3D11Device *GraphicsDevice::getDevice() const { return m_device.Get(); }
    ID3D11DeviceContext *GraphicsDevice::getDeviceContext() const { return m_context.Get(); }
}