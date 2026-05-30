#include "renderer/CascadeShadowMap.h"
#include "core/Logger.h"
#include <cmath>

using namespace DirectX;

namespace engine::renderer {

    CascadeShadowMap::CascadeShadowMap(
        ID3D11Device*               device,
        ID3D11DeviceContext*        context,
        const CascadeShadowMapDesc& desc
    )
        : m_context(context)
        , m_desc(desc)
    {
        LOG_DEBUG("Creating cascade shadow map (" + std::to_string(CASCADE_COUNT) + " cascades)...");

        ShadowMapDesc smDesc;
        smDesc.width  = desc.textureSize;
        smDesc.height = desc.textureSize;

        ShadowPassDesc spDesc;
        spDesc.sceneRadius    = desc.sceneRadius;
        spDesc.shadowDistance = desc.shadowDistance;
        spDesc.nearZ          = desc.nearZ;
        spDesc.farZ           = desc.farZ;
        spDesc.constantBias   = desc.constantBias;
        spDesc.slopeBias      = desc.slopeBias;
        spDesc.biasClamp      = desc.biasClamp;
        spDesc.dynamicBias    = desc.dynamicBias;

        for (int i = 0; i < CASCADE_COUNT; ++i) {
            m_cascades[i] = std::make_unique<ShadowMap>(device, smDesc);
            m_passes[i]   = std::make_unique<ShadowPass>(device, context, m_cascades[i].get(), spDesc);
        }

        m_cascadeCB = std::make_unique<ConstantBuffer<CascadeData>>(device, context);

        computeSplitDistances(desc.nearZ, desc.farZ);

        LOG_INFO("Cascade shadow map created");
    }

    void CascadeShadowMap::computeSplitDistances(float nearZ, float farZ) {
        float lambda = m_desc.lambda;

        for (int i = 0; i < CASCADE_COUNT; ++i) {
            float p         = static_cast<float>(i + 1) / static_cast<float>(CASCADE_COUNT);
            float log       = nearZ * powf(farZ / nearZ, p);
            float uniform   = nearZ + (farZ - nearZ) * p;
            float d         = lambda * (log - uniform) + uniform;
            m_cascadeData.splitDistances[i] = d;
        }
    }

    void CascadeShadowMap::computeLightSpaceMatrix(
        int             index,
        const Camera*   camera,
        const XMFLOAT3& lightDir,
        const XMFLOAT3& sceneCenter
    ) {
        float nearSplit = (index == 0)
            ? m_desc.nearZ
            : m_cascadeData.splitDistances[index - 1];
        float farSplit = m_cascadeData.splitDistances[index];

        float halfSize = farSplit * 0.5f;

        XMVECTOR dir    = XMVector3Normalize(XMLoadFloat3(&lightDir));
        XMVECTOR center = XMLoadFloat3(&sceneCenter);
        XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        if (fabsf(XMVectorGetY(dir)) > 0.99f)
            up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

        XMVECTOR lightPos = XMVectorSubtract(center, XMVectorScale(dir, m_desc.shadowDistance));

        XMMATRIX lightView = XMMatrixLookAtLH(lightPos, center, up);
        XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(
            -halfSize, halfSize, -halfSize, halfSize,
            m_desc.nearZ, m_desc.farZ
        );

        m_cascadeData.lightSpaceMatrices[index] = XMMatrixMultiply(lightView, lightProj);
    }

    void CascadeShadowMap::update(
        const Camera*   camera,
        const XMFLOAT3& lightDir,
        const XMFLOAT3& sceneCenter
    ) {
        XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&m_cascadeData.lightDir), 
                      XMVector3Normalize(XMLoadFloat3(&lightDir)));
        m_cascadeData.shadowBias = m_desc.dynamicBias;

        for (int i = 0; i < CASCADE_COUNT; ++i)
            computeLightSpaceMatrix(i, camera, lightDir, sceneCenter);

        m_cascadeCB->update(m_cascadeData);

        LOG_DEBUG("CSM updated");
    }

    void CascadeShadowMap::beginCascade(int index, GraphicsDevice* gfx) {
        m_cascades[index]->bindAsTarget(m_context);
        m_context->RSSetState(nullptr);
        LOG_DEBUG("CSM cascade " + std::to_string(index) + " begin");
    }

    void CascadeShadowMap::endCascade(GraphicsDevice* gfx, int viewportWidth, int viewportHeight) {
        D3D11_VIEWPORT vp = {};
        vp.Width    = static_cast<float>(viewportWidth);
        vp.Height   = static_cast<float>(viewportHeight);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_context->RSSetViewports(1, &vp);
        LOG_DEBUG("CSM cascade end");
    }

    void CascadeShadowMap::bindCascades(ID3D11DeviceContext* context, uint32_t srvSlotStart) {
        for (int i = 0; i < CASCADE_COUNT; ++i)
            m_cascades[i]->bindAsResource(context, srvSlotStart + i);
    }

    void CascadeShadowMap::unbindCascades(ID3D11DeviceContext* context, uint32_t srvSlotStart) {
        for (int i = 0; i < CASCADE_COUNT; ++i)
            m_cascades[i]->unbindAsResource(context, srvSlotStart + i);
    }

    ConstantBuffer<CascadeData>* CascadeShadowMap::getCascadeCB() {
        return m_cascadeCB.get();
    }

    int CascadeShadowMap::getCascadeCount() const {
        return CASCADE_COUNT;
    }
}