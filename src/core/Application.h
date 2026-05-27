#pragma once
#include "platform/Window.h"
#include "renderer/GraphicsDevice.h"
#include "renderer/Mesh.h"
#include "renderer/Shader.h"
#include "renderer/Camera.h"
#include "renderer/ConstantBuffer.h"
#include "renderer/TransformData.h"
#include "renderer/Texture2D.h"
#include "renderer/SamplerState.h"
#include "renderer/Material.h"
#include "renderer/Light.h"
#include "renderer/CubemapTexture.h"
#include "renderer/SkyboxBuffer.h"
#include "renderer/SkyboxMesh.h"
#include "renderer/DepthPrePass.h"
#include "renderer/OcclusionQuery.h"
#include "renderer/ShadowMap.h"
#include "renderer/ShadowPass.h"
#include "renderer/ShadowSampler.h"
#include "renderer/RenderTarget.h"
#include "renderer/PostProcessPass.h"
#include "renderer/PostProcessBuffer.h"
#include "renderer/FXAABuffer.h"
#include "renderer/BloomBuffer.h"
#include "renderer/SSAOBuffer.h"
#include "core/Scene.h"
#include "core/Transform.h"
#include "core/SceneHierarchy.h"
#include "core/MeshRenderer.h"
#include "core/BoundingBoxComponent.h"
#include "core/LodComponent.h"
#include "math/Frustum.h"
#include <memory>

namespace engine::core {
    class Application {
    public:
        Application();
        ~Application() = default;

        Application(const Application&)            = delete;
        Application& operator=(const Application&) = delete;

        int run();

    protected:
        virtual void onUpdate(float deltaTime) {}
        virtual void onRender()               {}

        std::unique_ptr<platform::Window>                                   m_window;
        std::unique_ptr<renderer::GraphicsDevice>                           m_graphics;
        std::unique_ptr<renderer::Mesh>                                     m_mesh;
        std::unique_ptr<renderer::Shader>                                   m_shader;
        std::unique_ptr<renderer::Camera>                                   m_camera;
        std::unique_ptr<renderer::ConstantBuffer<renderer::TransformData>>  m_transformCB;
        std::unique_ptr<renderer::ConstantBuffer<renderer::LightBuffer>>    m_lightCB;
        std::unique_ptr<renderer::Texture2D>                                m_diffuseTexture;
        std::unique_ptr<renderer::Texture2D>                                m_specularTexture;
        std::unique_ptr<renderer::Texture2D>                                m_normalMap;
        std::unique_ptr<renderer::SamplerState>                             m_sampler;

        std::unique_ptr<renderer::SkyboxMesh>                               m_skyboxMesh;
        std::unique_ptr<renderer::Shader>                                   m_skyboxShader;
        std::unique_ptr<renderer::CubemapTexture>                           m_skyboxTexture;
        std::unique_ptr<renderer::ConstantBuffer<renderer::SkyboxData>>     m_skyboxCB;
        std::unique_ptr<renderer::SamplerState>                             m_skyboxSampler;

        std::unique_ptr<renderer::DepthPrePass>                             m_depthPrePass;
        std::unique_ptr<renderer::OcclusionQuery>                           m_occlusionQuery;

        std::unique_ptr<renderer::ShadowMap>                                m_shadowMap;
        std::unique_ptr<renderer::ShadowPass>                               m_shadowPass;
        std::unique_ptr<renderer::ShadowSampler>                            m_shadowSampler;
        std::unique_ptr<renderer::Shader>                                   m_shadowShader;

        std::unique_ptr<renderer::RenderTarget>                                       m_sceneRT;
        std::unique_ptr<renderer::RenderTarget>                                       m_msaaRT;
        bool                                                                          m_msaaEnabled = true;
        std::unique_ptr<renderer::PostProcessPass>                                    m_blitPass;
        std::unique_ptr<renderer::ConstantBuffer<renderer::PostProcessData>>          m_postProcessCB;
        renderer::PostProcessData                                                     m_postProcessData;

        std::unique_ptr<renderer::RenderTarget>                                       m_ldrRT;
        std::unique_ptr<renderer::PostProcessPass>                                    m_fxaaPass;
        std::unique_ptr<renderer::ConstantBuffer<renderer::FXAAData>>                 m_fxaaCB;
        renderer::FXAAData                                                            m_fxaaData;

        std::unique_ptr<renderer::RenderTarget>                                       m_normalsRT;
        std::unique_ptr<renderer::RenderTarget>                                       m_bloomBrightRT;
        std::unique_ptr<renderer::RenderTarget>                                       m_bloomBlurHRT;
        std::unique_ptr<renderer::RenderTarget>                                       m_bloomBlurRT;
        std::unique_ptr<renderer::RenderTarget>                                       m_bloomCompositeRT;
        std::unique_ptr<renderer::PostProcessPass>                                    m_bloomBrightPass;
        std::unique_ptr<renderer::PostProcessPass>                                    m_bloomBlurHPass;
        std::unique_ptr<renderer::PostProcessPass>                                    m_bloomBlurVPass;
        std::unique_ptr<renderer::PostProcessPass>                                    m_bloomCompositePass;
        std::unique_ptr<renderer::ConstantBuffer<renderer::BloomData>>                m_bloomCB;
        std::unique_ptr<renderer::ConstantBuffer<renderer::BlurData>>                 m_blurCB;
        renderer::BloomData                                                           m_bloomData;
        renderer::BlurData                                                            m_blurData;
        bool                                                                          m_bloomEnabled = true;

        std::unique_ptr<renderer::RenderTarget>                                       m_ssaoRT;
        std::unique_ptr<renderer::RenderTarget>                                       m_ssaoBlurRT;
        std::unique_ptr<renderer::PostProcessPass>                                    m_ssaoPass;
        std::unique_ptr<renderer::PostProcessPass>                                    m_ssaoBlurPass;
        std::unique_ptr<renderer::ConstantBuffer<renderer::SSAOData>>                 m_ssaoCB;
        std::unique_ptr<renderer::ConstantBuffer<renderer::SSAOBlurData>>             m_ssaoBlurCB;
        renderer::SSAOData                                                            m_ssaoData;
        renderer::SSAOBlurData                                                        m_ssaoBlurData;
        bool                                                                          m_ssaoEnabled = true;
        Microsoft::WRL::ComPtr<ID3D11Texture2D>                                       m_ssaoNoiseTex;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                              m_ssaoNoiseSRV;
        std::unique_ptr<renderer::SamplerState>                                       m_ssaoNoiseSampler;
        std::unique_ptr<renderer::SamplerState>                                       m_ssaoClampSampler;

        std::unique_ptr<Scene>                                              m_scene;
        std::unique_ptr<SceneHierarchy>                                     m_hierarchy;
        std::unique_ptr<renderer::Material>                                 m_cubeMaterial;

        math::Frustum                                                       m_frustum;

        Entity*                                                             m_cubeEntity = nullptr;
    };
}