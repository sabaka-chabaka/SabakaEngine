#include "renderer/ShadowPass.h"
#include "core/Logger.h"
#include <stdexcept>

using namespace DirectX;

namespace engine::renderer {

    ShadowPass::ShadowPass(
        ID3D11Device*        device,
        ID3D11DeviceContext* context,
        ShadowMap*           shadowMap,
        const ShadowPassDesc& desc
    )
        : m_context(context)
        , m_shadowMap(shadowMap)
        , m_desc(desc)
    {
        LOG_DEBUG("Creating shadow pass...");

        m_shadowCB = std::make_unique<ConstantBuffer<ShadowData>>(device, context);

        D3D11_RASTERIZER_DESC rsDesc = {};
        rsDesc.FillMode              = D3D11_FILL_SOLID;
        rsDesc.CullMode              = D3D11_CULL_BACK;
        rsDesc.DepthBias             = static_cast<INT>(desc.constantBias);
        rsDesc.SlopeScaledDepthBias  = desc.slopeBias;
        rsDesc.DepthBiasClamp        = desc.biasClamp;
        rsDesc.DepthClipEnable       = TRUE;

        if (FAILED(device->CreateRasterizerState(&rsDesc, &m_shadowRasterizer)))
            throw std::runtime_error("ShadowPass: failed to create rasterizer state");

        LOG_INFO("Shadow pass created");
    }

    void ShadowPass::begin(
        GraphicsDevice*     gfx,
        const XMFLOAT3&     lightDir,
        const XMFLOAT3&     sceneCenter
    ) {
        XMVECTOR dir    = XMVector3Normalize(XMLoadFloat3(&lightDir));
        XMVECTOR center = XMLoadFloat3(&sceneCenter);

        XMVECTOR lightPos = center - dir * m_desc.shadowDistance;

        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        if (fabsf(XMVectorGetY(dir)) > 0.99f)
            up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

        XMMATRIX lightView = XMMatrixLookAtLH(lightPos, center, up);

        float half = m_desc.sceneRadius;
        XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(
            -half, half, -half, half,
            m_desc.nearZ, m_desc.farZ
        );

        m_shadowData.lightSpaceMatrix = XMMatrixTranspose(lightView * lightProj);
        XMStoreFloat3(&m_shadowData.lightDir, dir);
        m_shadowData.shadowBias = m_desc.dynamicBias;

        m_shadowCB->update(m_shadowData);

        m_shadowMap->bindAsTarget(m_context);
    }

    void ShadowPass::end(GraphicsDevice* gfx, int viewportWidth, int viewportHeight) {
    }

    ConstantBuffer<ShadowData>* ShadowPass::getShadowCB() {
        return m_shadowCB.get();
    }

    const ShadowData& ShadowPass::getShadowData() const {
        return m_shadowData;
    }
}
