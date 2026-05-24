#pragma once
#include "renderer/ShadowMap.h"
#include "renderer/ShadowPass.h"
#include "renderer/ConstantBuffer.h"
#include "renderer/GraphicsDevice.h"
#include "renderer/Camera.h"
#include <DirectXMath.h>
#include <array>
#include <memory>

namespace engine::renderer {
    using namespace DirectX;

    static constexpr int CASCADE_COUNT = 4;

    struct CascadeData {
        XMMATRIX lightSpaceMatrices[CASCADE_COUNT];
        float    splitDistances[CASCADE_COUNT];
        float3   lightDir;
        float    shadowBias;
    };

    struct CascadeShadowMapDesc {
        uint32_t textureSize    = 2048;
        float    sceneRadius    = 20.0f;
        float    shadowDistance = 200.0f;
        float    lambda         = 0.85f;
        float    nearZ          = 1.0f;
        float    farZ           = 200.0f;
        float    constantBias   = 100.0f;
        float    slopeBias      = 2.0f;
        float    biasClamp      = 0.1f;
        float    dynamicBias    = 0.005f;
    };

    class CascadeShadowMap {
    public:
        CascadeShadowMap(ID3D11Device* device, ID3D11DeviceContext* context,
                         const CascadeShadowMapDesc& desc = {});
        ~CascadeShadowMap() = default;

        CascadeShadowMap(const CascadeShadowMap&)            = delete;
        CascadeShadowMap& operator=(const CascadeShadowMap&) = delete;

        void update(
            const Camera*   camera,
            const XMFLOAT3& lightDir,
            const XMFLOAT3& sceneCenter
        );

        void beginCascade(int index, GraphicsDevice* gfx);
        void endCascade(GraphicsDevice* gfx, int viewportWidth, int viewportHeight);

        void bindCascades(ID3D11DeviceContext* context, uint32_t srvSlotStart);
        void unbindCascades(ID3D11DeviceContext* context, uint32_t srvSlotStart);

        ConstantBuffer<CascadeData>* getCascadeCB();
        int                          getCascadeCount() const;

    private:
        void computeSplitDistances(float nearZ, float farZ);
        void computeLightSpaceMatrix(int index, const Camera* camera,
                                     const XMFLOAT3& lightDir, const XMFLOAT3& sceneCenter);

        ID3D11DeviceContext*                              m_context;
        CascadeShadowMapDesc                              m_desc;

        std::array<std::unique_ptr<ShadowMap>, CASCADE_COUNT> m_cascades;
        std::array<std::unique_ptr<ShadowPass>, CASCADE_COUNT> m_passes;

        std::unique_ptr<ConstantBuffer<CascadeData>>      m_cascadeCB;

        CascadeData                                       m_cascadeData;
    };
}