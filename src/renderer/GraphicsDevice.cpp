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

        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };
        D3D_FEATURE_LEVEL featureLevel;

        UINT flags = 0;
#ifdef _DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            flags, featureLevels, ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION, &scd,
            &m_swapChain, &m_device, &featureLevel, &m_context
        );
        if (FAILED(hr)) {
            LOG_FATAL("Failed to create D3D11 device and swap chain");
            throw std::runtime_error("Failed to create D3D11 device");
        }

        {
            const char* levelStr = "Unknown";
            if (featureLevel == D3D_FEATURE_LEVEL_11_1) levelStr = "11.1";
            else if (featureLevel == D3D_FEATURE_LEVEL_11_0) levelStr = "11.0";
            LOG_INFO("D3D11 Device created. Feature Level: " + std::string(levelStr));

            ComPtr<IDXGIDevice> dxgiDevice;
            if (SUCCEEDED(m_device.As(&dxgiDevice))) {
                ComPtr<IDXGIAdapter> adapter;
                if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter))) {
                    DXGI_ADAPTER_DESC desc;
                    adapter->GetDesc(&desc);
                    
                    char sDesc[128];
                    size_t convertedChars = 0;
                    wcstombs_s(&convertedChars, sDesc, sizeof(sDesc), desc.Description, _TRUNCATE);
                    
                    LOG_INFO("Adapter: " + std::string(sDesc));
                    LOG_INFO("Video Memory: " + std::to_string(desc.DedicatedVideoMemory / (1024 * 1024)) + " MB");
                }
            }
        }

        createRenderTargetView();
        createDepthStencilBuffer(m_width, m_height);
        rebuildRasterizerState();
        rebuildBlendState();

        LOG_INFO("GraphicsDevice initialized successfully");
    }

    void GraphicsDevice::createRenderTargetView() {
        ComPtr<ID3D11Texture2D> backBuffer;
        if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
            throw std::runtime_error("Failed to get back buffer");

        if (FAILED(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView)))
            throw std::runtime_error("Failed to create RTV");
    }

    void GraphicsDevice::releaseRenderTargetView() {
        m_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_renderTargetView.Reset();
    }

    void GraphicsDevice::createDepthStencilBuffer(int width, int height) {
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = static_cast<UINT>(width);
        texDesc.Height = static_cast<UINT>(height);
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        if (FAILED(m_device->CreateTexture2D(&texDesc, nullptr, &m_depthStencilTexture)))
            throw std::runtime_error("Failed to create depth texture");

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;

        if (FAILED(m_device->CreateDepthStencilView(
            m_depthStencilTexture.Get(), &dsvDesc, &m_depthStencilView)))
            throw std::runtime_error("Failed to create DSV");

        LOG_DEBUG("Depth stencil buffer created (" + std::to_string(width) + "x" + std::to_string(height) + ")");

        rebuildDepthStencilState();
    }

    void GraphicsDevice::releaseDepthStencilBuffer() {
        m_depthStencilView.Reset();
        m_depthStencilTexture.Reset();
        m_depthStencilState.Reset();
    }

    void GraphicsDevice::rebuildDepthStencilState() {
        D3D11_DEPTH_STENCIL_DESC dsDesc = {};
        dsDesc.DepthEnable = TRUE;
        dsDesc.DepthWriteMask = m_depthWrite
                                    ? D3D11_DEPTH_WRITE_MASK_ALL
                                    : D3D11_DEPTH_WRITE_MASK_ZERO;
        dsDesc.DepthFunc = (m_depthFunc == DepthFunc::LessEqual)
                               ? D3D11_COMPARISON_LESS_EQUAL
                               : D3D11_COMPARISON_LESS;
        dsDesc.StencilEnable = FALSE;

        m_depthStencilState.Reset();
        if (FAILED(m_device->CreateDepthStencilState(&dsDesc, &m_depthStencilState)))
            throw std::runtime_error("Failed to create depth stencil state");
    }

    void GraphicsDevice::rebuildRasterizerState() {
        D3D11_RASTERIZER_DESC rsDesc = {};
        rsDesc.FillMode = (m_fillMode == FillMode::Wireframe)
                              ? D3D11_FILL_WIREFRAME
                              : D3D11_FILL_SOLID;
        rsDesc.CullMode = (m_cullMode == CullMode::None)
                              ? D3D11_CULL_NONE
                              : (m_cullMode == CullMode::Front)
                                    ? D3D11_CULL_FRONT
                                    : D3D11_CULL_BACK;
        rsDesc.FrontCounterClockwise = FALSE;
        rsDesc.DepthClipEnable = m_depthClip ? TRUE : FALSE;

        m_rasterizerState.Reset();
        if (FAILED(m_device->CreateRasterizerState(&rsDesc, &m_rasterizerState)))
            throw std::runtime_error("Failed to create rasterizer state");
    }

    void GraphicsDevice::rebuildBlendState() {
        D3D11_BLEND_DESC desc = {};
        auto &rt = desc.RenderTarget[0];
        rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        switch (m_blendMode) {
            case BlendMode::Opaque:
                rt.BlendEnable = FALSE;
                break;
            case BlendMode::AlphaBlend:
                rt.BlendEnable = TRUE;
                rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
                rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
                rt.BlendOp = D3D11_BLEND_OP_ADD;
                rt.SrcBlendAlpha = D3D11_BLEND_ONE;
                rt.DestBlendAlpha = D3D11_BLEND_ZERO;
                rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
                break;
            case BlendMode::Additive:
                rt.BlendEnable = TRUE;
                rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
                rt.DestBlend = D3D11_BLEND_ONE;
                rt.BlendOp = D3D11_BLEND_OP_ADD;
                rt.SrcBlendAlpha = D3D11_BLEND_ONE;
                rt.DestBlendAlpha = D3D11_BLEND_ZERO;
                rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
                break;
            case BlendMode::Multiplicative:
                rt.BlendEnable = TRUE;
                rt.SrcBlend = D3D11_BLEND_DEST_COLOR;
                rt.DestBlend = D3D11_BLEND_ZERO;
                rt.BlendOp = D3D11_BLEND_OP_ADD;
                rt.SrcBlendAlpha = D3D11_BLEND_ONE;
                rt.DestBlendAlpha = D3D11_BLEND_ZERO;
                rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
                break;
        }

        m_blendState.Reset();
        if (FAILED(m_device->CreateBlendState(&desc, &m_blendState)))
            throw std::runtime_error("Failed to create blend state");
    }

    void GraphicsDevice::beginFrame(float r, float g, float b) {
        m_context->OMSetRenderTargets(
            1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
        m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

        float blendFactor[4] = {};
        m_context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xffffffff);
        m_context->RSSetState(m_rasterizerState.Get());

        float color[4] = {r, g, b, 1.0f};
        m_context->ClearRenderTargetView(m_renderTargetView.Get(), color);
        m_context->ClearDepthStencilView(
            m_depthStencilView.Get(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f, 0
        );

        D3D11_VIEWPORT vp = {};
        vp.Width = static_cast<float>(m_width);
        vp.Height = static_cast<float>(m_height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_context->RSSetViewports(1, &vp);
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

        if (FAILED(m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0)))
            throw std::runtime_error("Failed to resize swap chain");

        createRenderTargetView();
        createDepthStencilBuffer(width, height);

        LOG_DEBUG("Swap chain resized to " +
            std::to_string(width) + "x" + std::to_string(height));
    }

    void GraphicsDevice::setFillMode(FillMode mode) {
        m_fillMode = mode;
        rebuildRasterizerState();
        m_context->RSSetState(m_rasterizerState.Get());
    }

    void GraphicsDevice::setCullMode(CullMode mode) {
        m_cullMode = mode;
        rebuildRasterizerState();
        m_context->RSSetState(m_rasterizerState.Get());
    }

    void GraphicsDevice::setBlendMode(BlendMode mode) {
        m_blendMode = mode;
        rebuildBlendState();
        float blendFactor[4] = {};
        m_context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xffffffff);
    }

    void GraphicsDevice::setDepthWriteEnabled(bool enabled) {
        m_depthWrite = enabled;
        rebuildDepthStencilState();
        m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    }

    void GraphicsDevice::setDepthFunc(DepthFunc func) {
        m_depthFunc = func;
        rebuildDepthStencilState();
        m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    }

    void GraphicsDevice::setDepthClipEnabled(bool enabled) {
        m_depthClip = enabled;
        rebuildRasterizerState();
        m_context->RSSetState(m_rasterizerState.Get());
    }

    ID3D11Device *GraphicsDevice::getDevice() const { return m_device.Get(); }
    ID3D11DeviceContext *GraphicsDevice::getDeviceContext() const { return m_context.Get(); }
}