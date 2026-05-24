#pragma once
#include "ShadowMap.h"
#include "ConstantBuffer.h"
#include "GraphicsDevice.h"
#include "Camera.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>

namespace engine::renderer {
    using namespace DirectX;
    using Microsoft::WRL::ComPtr;

    struct ShadowData {
        XMMATRIX lightSpaceMatrix;
        XMFLOAT3 lightDir;
        float    shadowBias;
    };

    struct ShadowPassDesc {
        float    sceneRadius    = 20.0f;
        float    shadowDistance = 50.0f;
        float    nearZ          = 1.0f;
        float    farZ           = 200.0f;
        float    constantBias   = 100.0f;
        float    slopeBias      = 2.0f;
        float    biasClamp      = 0.1f;
        float    dynamicBias    = 0.005f;
    };

    class ShadowPass {
    public:
        ShadowPass(
            ID3D11Device*        device,
            ID3D11DeviceContext* context,
            ShadowMap*           shadowMap,
            const ShadowPassDesc& desc = {}
        );

        ~ShadowPass() = default;

        ShadowPass(const ShadowPass&)            = delete;
        ShadowPass& operator=(const ShadowPass&) = delete;

        void begin(GraphicsDevice* gfx, const XMFLOAT3& lightDir, const XMFLOAT3& sceneCenter);
        void end(GraphicsDevice* gfx, int viewportWidth, int viewportHeight);

        ConstantBuffer<ShadowData>* getShadowCB();
        const ShadowData&           getShadowData() const;

    private:
        ID3D11DeviceContext*                    m_context;
        ShadowMap*                              m_shadowMap;
        ShadowPassDesc                          m_desc;

        std::unique_ptr<ConstantBuffer<ShadowData>> m_shadowCB;

        ComPtr<ID3D11RasterizerState>           m_shadowRasterizer;

        ShadowData                              m_shadowData;
    };
}
