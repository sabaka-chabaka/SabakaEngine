#include "PostProcessPass.h"
#include "core/Logger.h"

namespace engine::renderer {

    PostProcessPass::PostProcessPass(
        ID3D11Device*        device,
        ID3D11DeviceContext* context,
        const std::wstring&  psPath
    )
        : m_context(context)
    {
        LOG_DEBUG("Creating post process pass...");

        m_shader = std::make_unique<Shader>(
            device,
            L"shaders/Fullscreen.vs.hlsl",
            psPath,
            std::vector<InputElementDesc>{}
        );

        SamplerDesc sampDesc;
        sampDesc.filter = FilterMode::Bilinear;
        sampDesc.wrapU  = WrapMode::Clamp;
        sampDesc.wrapV  = WrapMode::Clamp;
        m_sampler = std::make_unique<SamplerState>(device, context, sampDesc);

        LOG_DEBUG("Post process pass created");
    }

    void PostProcessPass::render(RenderTarget* input, RenderTarget* output, GraphicsDevice* gfx) {
        ID3D11RenderTargetView* nullRTV = nullptr;
        m_context->OMSetRenderTargets(1, &nullRTV, nullptr);

        ID3D11RenderTargetView* rtv = output->getRTV();
        m_context->OMSetRenderTargets(1, &rtv, nullptr);

        float clear[4] = { 0, 0, 0, 1 };
        m_context->ClearRenderTargetView(rtv, clear);

        D3D11_VIEWPORT vp = {};
        vp.Width    = static_cast<float>(output->getWidth());
        vp.Height   = static_cast<float>(output->getHeight());
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_context->RSSetViewports(1, &vp);

        input->bindSRV(m_context, 0);
        m_sampler->bindPS(0);
        m_shader->bind(m_context);
        drawFullscreenTriangle();
        input->unbindSRV(m_context, 0);
    }

    void PostProcessPass::renderToBackBuffer(RenderTarget* input, GraphicsDevice* gfx) {
        ID3D11RenderTargetView* nullRTV = nullptr;
        m_context->OMSetRenderTargets(1, &nullRTV, nullptr);

        ID3D11RenderTargetView* backRTV = gfx->getRTV();
        m_context->OMSetRenderTargets(1, &backRTV, nullptr);

        D3D11_VIEWPORT vp = {};
        vp.Width    = static_cast<float>(gfx->getWidth());
        vp.Height   = static_cast<float>(gfx->getHeight());
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_context->RSSetViewports(1, &vp);

        input->bindSRV(m_context, 0);
        m_sampler->bindPS(0);
        m_shader->bind(m_context);
        drawFullscreenTriangle();
        input->unbindSRV(m_context, 0);
    }

    void PostProcessPass::drawFullscreenTriangle() {
        m_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        m_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->Draw(3, 0);
    }

    Shader* PostProcessPass::getShader() { return m_shader.get(); }
}