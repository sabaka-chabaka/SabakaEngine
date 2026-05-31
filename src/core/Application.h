#pragma once
#include "platform/Window.h"
#include "renderer/GraphicsDevice.h"
#include "renderer/Camera.h"
#include "renderer/ConstantBuffer.h"
#include "renderer/TransformData.h"
#include "renderer/Light.h"
#include "renderer/RenderTarget.h"
#include "renderer/PostProcessPass.h"
#include "renderer/PostProcessBuffer.h"
#include "renderer/FXAABuffer.h"
#include "renderer/BloomBuffer.h"
#include "renderer/SSAOBuffer.h"
#include "renderer/MotionBlurBuffer.h"
#include "renderer/DofBuffer.h"
#include "renderer/ColorGradingBuffer.h"
#include "renderer/SkyboxBuffer.h"
#include "renderer/SkyboxMesh.h"
#include "renderer/CubemapTexture.h"
#include "renderer/SamplerState.h"
#include "renderer/Shader.h"
#include "renderer/ShadowMap.h"
#include "renderer/ShadowPass.h"
#include "renderer/ShadowSampler.h"
#include "renderer/DepthPrePass.h"
#include "renderer/OcclusionQuery.h"
#include "assets/AssetManager.h"
#include "core/VFS.h"
#include "core/FileWatcher.h"
#include "core/Scene.h"
#include "core/Transform.h"
#include "core/SceneHierarchy.h"
#include "math/Frustum.h"
#include "physics/PhysicsWorld.h"
#include "audio/AudioEngine.h"
#include "audio/AudioMixer.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace engine::core {

    class Application {
    public:
        Application();
        virtual ~Application();

        Application(const Application&)            = delete;
        Application& operator=(const Application&) = delete;

        int run();

    protected:
        virtual void onInit()                  {}
        virtual void onUpdate(float deltaTime) {}
        virtual void onRender()                {}

        void initRenderPipeline();
        void renderFrame(float deltaTime);

        std::unique_ptr<platform::Window>                                   m_window;
        std::unique_ptr<renderer::GraphicsDevice>                           m_graphics;
        std::unique_ptr<renderer::Camera>                                   m_camera;
        std::unique_ptr<renderer::ConstantBuffer<renderer::TransformData>>  m_transformCB;
        std::unique_ptr<renderer::ConstantBuffer<renderer::LightBuffer>>    m_lightCB;

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

        std::unique_ptr<renderer::RenderTarget>                                       m_motionBlurRT;
        std::unique_ptr<renderer::PostProcessPass>                                    m_motionBlurPass;
        std::unique_ptr<renderer::ConstantBuffer<renderer::MotionBlurData>>           m_motionBlurCB;
        renderer::MotionBlurData                                                      m_motionBlurData;
        bool                                                                          m_motionBlurEnabled = true;
        DirectX::XMMATRIX                                                             m_prevViewProj;

        std::unique_ptr<renderer::RenderTarget>                                       m_dofRT;
        std::unique_ptr<renderer::PostProcessPass>                                    m_dofPass;
        std::unique_ptr<renderer::ConstantBuffer<renderer::DofData>>                  m_dofCB;
        renderer::DofData                                                             m_dofData;
        bool                                                                          m_dofEnabled = true;

        std::unique_ptr<renderer::RenderTarget>                                       m_colorGradingRT;
        std::unique_ptr<renderer::PostProcessPass>                                    m_colorGradingPass;
        std::unique_ptr<renderer::ConstantBuffer<renderer::ColorGradingData>>         m_colorGradingCB;
        renderer::ColorGradingData                                                    m_colorGradingData;
        bool                                                                          m_colorGradingEnabled = true;
        Microsoft::WRL::ComPtr<ID3D11Texture3D>                                       m_lutTex;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                              m_lutSRV;
        std::unique_ptr<renderer::SamplerState>                                       m_lutSampler;

        std::unique_ptr<renderer::RenderTarget>                                       m_vignetteRT;
        std::unique_ptr<renderer::PostProcessPass>                                    m_vignettePass;
        std::unique_ptr<renderer::ConstantBuffer<renderer::VignetteData>>             m_vignetteCB;
        renderer::VignetteData                                                        m_vignetteData;
        bool                                                                          m_vignetteEnabled   = true;
        bool                                                                          m_aberrationEnabled = true;

        std::unique_ptr<Scene>                                              m_scene;
        std::unique_ptr<SceneHierarchy>                                     m_hierarchy;
        std::unique_ptr<assets::AssetManager>                               m_assetManager;

        std::unique_ptr<FileWatcher>                                        m_shaderWatcher;
        std::unique_ptr<FileWatcher>                                        m_textureWatcher;

        std::mutex                                                          m_reloadMutex;
        std::vector<std::string>                                            m_pendingShaderReloads;
        std::vector<std::string>                                            m_pendingTextureReloads;

        math::Frustum                                                       m_frustum;

        std::unique_ptr<physics::PhysicsWorld>                              m_physicsWorld;

        std::unique_ptr<audio::AudioEngine>                                 m_audioEngine;
        std::unique_ptr<audio::AudioMixer>                                  m_audioMixer;

        renderer::LightBuffer                                               m_lightBuffer;
    };
}