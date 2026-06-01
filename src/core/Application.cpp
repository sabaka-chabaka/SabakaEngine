#define NOMINMAX
#include "core/Application.h"
#include "core/Logger.h"
#include "math/AABB.h"
#include "platform/Input.h"
#include "physics/RigidBody.h"
#include "audio/AudioEngine.h"
#include "audio/AudioMixer.h"


#include <DirectXMath.h>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <filesystem>

using namespace DirectX;
using namespace engine::platform;

namespace engine::core {

    static std::string getExeDir() {
        wchar_t exePath[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        return std::filesystem::path(exePath).parent_path().string();
    }

    Application::Application() {
        std::string exeDir = getExeDir();

        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

        Logger::get().openLogFile("engine.log");
        Logger::get().setMinLevel(LogLevel::Debug);

        LOG_INFO("Initializing SabakaEngine");

        platform::WindowDesc wDesc;
        wDesc.title  = L"SabakaEngine";
        wDesc.width  = 1280;
        wDesc.height = 720;

        LOG_DEBUG("Creating window...");
        m_window = std::make_unique<platform::Window>(wDesc);
        LOG_INFO("Window created: 1280x720");

        LOG_DEBUG("Initializing input system...");
        InputSystem::get().initialize(m_window->getNativeHandle());

        renderer::GraphicsDeviceDesc gDesc;
        gDesc.hwnd   = m_window->getNativeHandle();
        gDesc.width  = m_window->getWidth();
        gDesc.height = m_window->getHeight();
        gDesc.vsync  = true;

        LOG_DEBUG("Initializing GraphicsDevice...");
        m_graphics = std::make_unique<renderer::GraphicsDevice>(gDesc);
        LOG_INFO("DirectX 11 device initialized");

        m_window->setResizeCallback([this](int w, int h) {
            LOG_DEBUG("Window resized: " + std::to_string(w) + "x" + std::to_string(h));
            m_graphics->onResize(w, h);
            if (m_camera && h > 0)
                m_camera->setAspectRatio(static_cast<float>(w) / static_cast<float>(h));
            if (m_sceneRT && w > 0 && h > 0)
                m_sceneRT->resize(m_graphics->getDevice(), static_cast<uint32_t>(w), static_cast<uint32_t>(h));
            if (m_msaaRT && w > 0 && h > 0)
                m_msaaRT->resize(m_graphics->getDevice(), static_cast<uint32_t>(w), static_cast<uint32_t>(h));
            if (m_ldrRT && w > 0 && h > 0) {
                m_ldrRT->resize(m_graphics->getDevice(), static_cast<uint32_t>(w), static_cast<uint32_t>(h));
                m_fxaaData.texelSizeX = 1.0f / static_cast<float>(w);
                m_fxaaData.texelSizeY = 1.0f / static_cast<float>(h);
            }
            if (m_normalsRT && w > 0 && h > 0)
                m_normalsRT->resize(m_graphics->getDevice(), static_cast<uint32_t>(w), static_cast<uint32_t>(h));
            if (m_bloomBrightRT && w > 0 && h > 0) {
                m_bloomBrightRT->resize(m_graphics->getDevice(), static_cast<uint32_t>(w) / 2, static_cast<uint32_t>(h) / 2);
                m_bloomBlurHRT->resize(m_graphics->getDevice(),  static_cast<uint32_t>(w) / 2, static_cast<uint32_t>(h) / 2);
                m_bloomBlurRT->resize(m_graphics->getDevice(),   static_cast<uint32_t>(w) / 2, static_cast<uint32_t>(h) / 2);
                m_bloomCompositeRT->resize(m_graphics->getDevice(), static_cast<uint32_t>(w), static_cast<uint32_t>(h));
                m_blurData.texelSizeX = 2.0f / static_cast<float>(w);
                m_blurData.texelSizeY = 2.0f / static_cast<float>(h);
            }
            if (m_ssaoRT && w > 0 && h > 0) {
                m_ssaoRT->resize(m_graphics->getDevice(),     static_cast<uint32_t>(w), static_cast<uint32_t>(h));
                m_ssaoBlurRT->resize(m_graphics->getDevice(), static_cast<uint32_t>(w), static_cast<uint32_t>(h));
                m_ssaoData.noiseScaleX    = static_cast<float>(w) / static_cast<float>(renderer::SSAO_NOISE_SIZE);
                m_ssaoData.noiseScaleY    = static_cast<float>(h) / static_cast<float>(renderer::SSAO_NOISE_SIZE);
                m_ssaoBlurData.texelSizeX = 1.0f / static_cast<float>(w);
                m_ssaoBlurData.texelSizeY = 1.0f / static_cast<float>(h);
            }
            if (m_motionBlurRT && w > 0 && h > 0)
                m_motionBlurRT->resize(m_graphics->getDevice(), static_cast<uint32_t>(w), static_cast<uint32_t>(h));
            if (m_dofRT && w > 0 && h > 0)
                m_dofRT->resize(m_graphics->getDevice(), static_cast<uint32_t>(w), static_cast<uint32_t>(h));
            if (m_colorGradingRT && w > 0 && h > 0)
                m_colorGradingRT->resize(m_graphics->getDevice(), static_cast<uint32_t>(w), static_cast<uint32_t>(h));
            if (m_vignetteRT && w > 0 && h > 0)
                m_vignetteRT->resize(m_graphics->getDevice(), static_cast<uint32_t>(w), static_cast<uint32_t>(h));
        });

        initCoreSystems();
        initRenderPipeline();

        LOG_INFO("Engine systems ready");
    }

    void Application::initCoreSystems() {
        std::string exeDir = getExeDir();

        VFS::get().mount("assets",   exeDir);
        VFS::get().mount("shaders",  exeDir + "/shaders");
        VFS::get().mount("textures", exeDir + "/textures");
        VFS::get().mount("models",   exeDir + "/models");

        LOG_DEBUG("Starting shader FileWatcher...");
        m_shaderWatcher = std::make_unique<FileWatcher>(
            exeDir + "/shaders",
            [this](const FileChangeEvent& ev) {
                const std::string& p = ev.path;
                if (p.size() >= 5 && p.substr(p.size() - 5) == ".hlsl") {
                    std::unique_lock lock(m_reloadMutex);
                    m_pendingShaderReloads.push_back(p);
                }
            });

        LOG_DEBUG("Starting texture FileWatcher...");
        m_textureWatcher = std::make_unique<FileWatcher>(
            exeDir + "/textures",
            [this](const FileChangeEvent& ev) {
                const std::string& p = ev.path;
                auto ext = std::filesystem::path(p).extension().string();
                if (ext == ".png" || ext == ".jpg" || ext == ".dds" || ext == ".bmp") {
                    std::unique_lock lock(m_reloadMutex);
                    m_pendingTextureReloads.push_back(p);
                }
            });

        m_assetManager = std::make_unique<assets::AssetManager>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        LOG_DEBUG("Initializing PhysicsWorld...");
        m_physicsWorld = std::make_unique<physics::PhysicsWorld>();

        LOG_DEBUG("Initializing AudioEngine...");
        m_audioEngine = std::make_unique<audio::AudioEngine>();
        m_audioMixer  = std::make_unique<audio::AudioMixer>(m_audioEngine.get());

        m_scene     = std::make_unique<Scene>();
        m_hierarchy = std::make_unique<SceneHierarchy>();
    }

    Application::~Application() {
        LOG_INFO("Shutting down SabakaEngine");
    }

    void Application::initRenderPipeline(uint32_t width, uint32_t height) {
        uint32_t w = width  > 0 ? width  : static_cast<uint32_t>(m_window->getWidth());
        uint32_t h = height > 0 ? height : static_cast<uint32_t>(m_window->getHeight());

        renderer::CameraDesc camDesc;
        camDesc.position = { 0.0f, 1.5f, -15.0f };
        camDesc.target   = { 0.0f, 0.0f,   0.0f };
        camDesc.up       = { 0.0f, 1.0f,   0.0f };
        camDesc.fovY     = XM_PIDIV4;
        camDesc.nearZ    = 0.1f;
        camDesc.farZ     = 1000.0f;

        m_camera = std::make_unique<renderer::Camera>(camDesc);
        m_camera->setAspectRatio(static_cast<float>(w) / static_cast<float>(h));

        m_transformCB = std::make_unique<renderer::ConstantBuffer<renderer::TransformData>>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        m_lightCB = std::make_unique<renderer::ConstantBuffer<renderer::LightBuffer>>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        LOG_DEBUG("Creating skybox...");
        m_skyboxMesh = std::make_unique<renderer::SkyboxMesh>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        m_skyboxShader = std::make_unique<renderer::Shader>(
            m_graphics->getDevice(),
            L"shaders/Skybox.vs.hlsl",
            L"shaders/Skybox.ps.hlsl",
            std::vector<renderer::InputElementDesc>{});

        m_skyboxCB = std::make_unique<renderer::ConstantBuffer<renderer::SkyboxData>>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        renderer::SamplerDesc skyboxSampDesc;
        skyboxSampDesc.filter = renderer::FilterMode::Trilinear;
        skyboxSampDesc.wrapU  = renderer::WrapMode::Clamp;
        skyboxSampDesc.wrapV  = renderer::WrapMode::Clamp;
        m_skyboxSampler = std::make_unique<renderer::SamplerState>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), skyboxSampDesc);

        LOG_DEBUG("Creating depth pre-pass...");
        m_depthPrePass = std::make_unique<renderer::DepthPrePass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        LOG_DEBUG("Creating occlusion query...");
        m_occlusionQuery = std::make_unique<renderer::OcclusionQuery>(m_graphics->getDevice());

        LOG_DEBUG("Creating shadow map...");
        renderer::ShadowMapDesc smDesc;
        smDesc.width  = 2048;
        smDesc.height = 2048;
        m_shadowMap = std::make_unique<renderer::ShadowMap>(m_graphics->getDevice(), smDesc);

        std::vector<renderer::InputElementDesc> shadowLayout = {
            {"POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0},
        };
        m_shadowShader = std::make_unique<renderer::Shader>(
            m_graphics->getDevice(),
            L"shaders/Shadow.vs.hlsl",
            L"shaders/Shadow.ps.hlsl",
            shadowLayout);

        renderer::ShadowPassDesc spDesc;
        spDesc.sceneRadius    = 10.0f;
        spDesc.shadowDistance = 30.0f;
        spDesc.nearZ          = 1.0f;
        spDesc.farZ           = 100.0f;
        spDesc.constantBias   = 100.0f;
        spDesc.slopeBias      = 2.0f;
        spDesc.biasClamp      = 0.1f;
        spDesc.dynamicBias    = 0.005f;
        m_shadowPass = std::make_unique<renderer::ShadowPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(),
            m_shadowMap.get(), spDesc);

        m_shadowSampler = std::make_unique<renderer::ShadowSampler>(m_graphics->getDevice());

        LOG_DEBUG("Creating scene render targets...");
        {
            renderer::RenderTargetDesc rtDesc;
            rtDesc.width    = w;
            rtDesc.height   = h;
            rtDesc.format   = renderer::RenderTargetFormat::RGBA16_FLOAT;
            rtDesc.hasDepth = true;
            m_sceneRT = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), rtDesc);
        }
        {
            renderer::RenderTargetDesc msaaDesc;
            msaaDesc.width       = w;
            msaaDesc.height      = h;
            msaaDesc.format      = renderer::RenderTargetFormat::RGBA16_FLOAT;
            msaaDesc.hasDepth    = true;
            msaaDesc.sampleCount = 4;
            m_msaaRT = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), msaaDesc);
        }

        LOG_DEBUG("Creating post-process passes...");
        m_blitPass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), L"shaders/Blit.ps.hlsl");

        m_postProcessData.exposure    = 1.0f;
        m_postProcessData.tonemapMode = static_cast<int>(renderer::TonemapMode::Reinhard);
        m_postProcessData._pad0       = 0.0f;
        m_postProcessData._pad1       = 0.0f;
        m_postProcessCB = std::make_unique<renderer::ConstantBuffer<renderer::PostProcessData>>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        {
            renderer::RenderTargetDesc ldrDesc;
            ldrDesc.width    = w;
            ldrDesc.height   = h;
            ldrDesc.format   = renderer::RenderTargetFormat::RGBA8_UNORM;
            ldrDesc.hasDepth = false;
            m_ldrRT = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), ldrDesc);
        }

        m_fxaaPass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), L"shaders/FXAA.ps.hlsl");
        m_fxaaData.texelSizeX = 1.0f / static_cast<float>(w);
        m_fxaaData.texelSizeY = 1.0f / static_cast<float>(h);
        m_fxaaData.enabled    = 1;
        m_fxaaData._pad       = 0.0f;
        m_fxaaCB = std::make_unique<renderer::ConstantBuffer<renderer::FXAAData>>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        {
            renderer::RenderTargetDesc normDesc;
            normDesc.width       = w;
            normDesc.height      = h;
            normDesc.format      = renderer::RenderTargetFormat::RGBA16_FLOAT;
            normDesc.hasDepth    = false;
            normDesc.sampleCount = m_msaaEnabled ? 4 : 1;
            m_normalsRT = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), normDesc);
        }

        LOG_DEBUG("Creating bloom passes...");
        {
            renderer::RenderTargetDesc bloomDesc;
            bloomDesc.width    = w / 2;
            bloomDesc.height   = h / 2;
            bloomDesc.format   = renderer::RenderTargetFormat::RGBA16_FLOAT;
            bloomDesc.hasDepth = false;
            m_bloomBrightRT    = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), bloomDesc);
            m_bloomBlurHRT     = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), bloomDesc);
            m_bloomBlurRT      = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), bloomDesc);
        }
        {
            renderer::RenderTargetDesc compDesc;
            compDesc.width    = w;
            compDesc.height   = h;
            compDesc.format   = renderer::RenderTargetFormat::RGBA16_FLOAT;
            compDesc.hasDepth = false;
            m_bloomCompositeRT = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), compDesc);
        }
        m_bloomBrightPass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), L"shaders/BloomBright.ps.hlsl");
        m_bloomBlurHPass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), L"shaders/BloomBlur.ps.hlsl");
        m_bloomBlurVPass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), L"shaders/BloomBlur.ps.hlsl");
        m_bloomCompositePass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), L"shaders/BloomComposite.ps.hlsl");
        m_bloomData.threshold   = 1.0f;
        m_bloomData.intensity   = 0.5f;
        m_bloomData.ssaoEnabled = 1.0f;
        m_bloomData._pad        = 0.0f;
        m_bloomCB = std::make_unique<renderer::ConstantBuffer<renderer::BloomData>>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());
        m_blurData.texelSizeX = 2.0f / static_cast<float>(w);
        m_blurData.texelSizeY = 2.0f / static_cast<float>(h);
        m_blurData.horizontal = 1;
        m_blurData._pad       = 0.0f;
        m_blurCB = std::make_unique<renderer::ConstantBuffer<renderer::BlurData>>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        LOG_DEBUG("Creating SSAO passes...");
        {
            renderer::RenderTargetDesc ssaoDesc;
            ssaoDesc.width    = w;
            ssaoDesc.height   = h;
            ssaoDesc.format   = renderer::RenderTargetFormat::RGBA8_UNORM;
            ssaoDesc.hasDepth = false;
            m_ssaoRT     = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), ssaoDesc);
            m_ssaoBlurRT = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), ssaoDesc);
        }
        m_ssaoPass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), L"shaders/SSAO.ps.hlsl");
        m_ssaoBlurPass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), L"shaders/SSAOBlur.ps.hlsl");
        {
            auto noise = renderer::generateSSAONoise();
            D3D11_TEXTURE2D_DESC noiseDesc          = {};
            noiseDesc.Width                         = renderer::SSAO_NOISE_SIZE;
            noiseDesc.Height                        = renderer::SSAO_NOISE_SIZE;
            noiseDesc.MipLevels                     = 1;
            noiseDesc.ArraySize                     = 1;
            noiseDesc.Format                        = DXGI_FORMAT_R32G32B32A32_FLOAT;
            noiseDesc.SampleDesc.Count              = 1;
            noiseDesc.Usage                         = D3D11_USAGE_DEFAULT;
            noiseDesc.BindFlags                     = D3D11_BIND_SHADER_RESOURCE;
            D3D11_SUBRESOURCE_DATA noiseInit        = {};
            noiseInit.pSysMem                       = noise.data();
            noiseInit.SysMemPitch                   = renderer::SSAO_NOISE_SIZE * sizeof(XMFLOAT4);
            if (FAILED(m_graphics->getDevice()->CreateTexture2D(&noiseDesc, &noiseInit, &m_ssaoNoiseTex)))
                throw std::runtime_error("Failed to create SSAO noise texture");
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format                          = DXGI_FORMAT_R32G32B32A32_FLOAT;
            srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels             = 1;
            srvDesc.Texture2D.MostDetailedMip       = 0;
            if (FAILED(m_graphics->getDevice()->CreateShaderResourceView(m_ssaoNoiseTex.Get(), &srvDesc, &m_ssaoNoiseSRV)))
                throw std::runtime_error("Failed to create SSAO noise SRV");
        }
        {
            renderer::SamplerDesc wrapDesc;
            wrapDesc.filter = renderer::FilterMode::Point;
            wrapDesc.wrapU  = renderer::WrapMode::Repeat;
            wrapDesc.wrapV  = renderer::WrapMode::Repeat;
            m_ssaoNoiseSampler = std::make_unique<renderer::SamplerState>(
                m_graphics->getDevice(), m_graphics->getDeviceContext(), wrapDesc);
            renderer::SamplerDesc clampDesc;
            clampDesc.filter = renderer::FilterMode::Point;
            clampDesc.wrapU  = renderer::WrapMode::Clamp;
            clampDesc.wrapV  = renderer::WrapMode::Clamp;
            m_ssaoClampSampler = std::make_unique<renderer::SamplerState>(
                m_graphics->getDevice(), m_graphics->getDeviceContext(), clampDesc);
        }
        {
            auto sampleArr         = renderer::generateSSAOSamples();
            m_ssaoData.radius      = 0.5f;
            m_ssaoData.bias        = 0.025f;
            m_ssaoData.numSamples  = renderer::SSAO_NUM_SAMPLES;
            m_ssaoData.noiseScaleX = static_cast<float>(w) / static_cast<float>(renderer::SSAO_NOISE_SIZE);
            m_ssaoData.noiseScaleY = static_cast<float>(h) / static_cast<float>(renderer::SSAO_NOISE_SIZE);
            for (int i = 0; i < 64; ++i)
                m_ssaoData.samples[i] = sampleArr[i];
            m_ssaoCB = std::make_unique<renderer::ConstantBuffer<renderer::SSAOData>>(
                m_graphics->getDevice(), m_graphics->getDeviceContext());
        }
        m_ssaoBlurData.texelSizeX     = 1.0f / static_cast<float>(w);
        m_ssaoBlurData.texelSizeY     = 1.0f / static_cast<float>(h);
        m_ssaoBlurData.depthThreshold = 0.01f;
        m_ssaoBlurData._pad           = 0.0f;
        m_ssaoBlurCB = std::make_unique<renderer::ConstantBuffer<renderer::SSAOBlurData>>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        LOG_DEBUG("Creating motion blur pass...");
        {
            renderer::RenderTargetDesc mbDesc;
            mbDesc.width    = w;
            mbDesc.height   = h;
            mbDesc.format   = renderer::RenderTargetFormat::RGBA16_FLOAT;
            mbDesc.hasDepth = false;
            m_motionBlurRT = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), mbDesc);
        }
        m_motionBlurPass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), L"shaders/MotionBlur.ps.hlsl");
        m_motionBlurData.prevViewProj = XMMatrixIdentity();
        m_motionBlurData.invViewProj  = XMMatrixIdentity();
        m_motionBlurData.strength     = 1.0f;
        m_motionBlurData.numSamples   = 8;
        m_motionBlurData.enabled      = 1;
        m_motionBlurData._pad         = 0.0f;
        m_motionBlurCB = std::make_unique<renderer::ConstantBuffer<renderer::MotionBlurData>>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());
        m_prevViewProj = XMMatrixIdentity();

        LOG_DEBUG("Creating DoF pass...");
        {
            renderer::RenderTargetDesc dofDesc;
            dofDesc.width    = w;
            dofDesc.height   = h;
            dofDesc.format   = renderer::RenderTargetFormat::RGBA16_FLOAT;
            dofDesc.hasDepth = false;
            m_dofRT = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), dofDesc);
        }
        m_dofPass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), L"shaders/DoF.ps.hlsl");
        m_dofData.focusDistance = 15.0f;
        m_dofData.focusRange    = 5.0f;
        m_dofData.maxBlurRadius = 8.0f;
        m_dofData.enabled       = 1;
        m_dofData.nearZ         = 0.1f;
        m_dofData.farZ          = 1000.0f;
        m_dofData.numSamples    = 16;
        m_dofData._pad          = 0.0f;
        m_dofCB = std::make_unique<renderer::ConstantBuffer<renderer::DofData>>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        LOG_DEBUG("Creating Color Grading pass...");
        {
            static constexpr int LUT_SIZE = 16;
            std::vector<XMFLOAT4> lutData(LUT_SIZE * LUT_SIZE * LUT_SIZE);
            for (int b = 0; b < LUT_SIZE; ++b)
                for (int g = 0; g < LUT_SIZE; ++g)
                    for (int r = 0; r < LUT_SIZE; ++r) {
                        int idx        = b * LUT_SIZE * LUT_SIZE + g * LUT_SIZE + r;
                        lutData[idx].x = static_cast<float>(r) / (LUT_SIZE - 1);
                        lutData[idx].y = static_cast<float>(g) / (LUT_SIZE - 1);
                        lutData[idx].z = static_cast<float>(b) / (LUT_SIZE - 1);
                        lutData[idx].w = 1.0f;
                    }
            D3D11_TEXTURE3D_DESC lutDesc  = {};
            lutDesc.Width                 = LUT_SIZE;
            lutDesc.Height                = LUT_SIZE;
            lutDesc.Depth                 = LUT_SIZE;
            lutDesc.MipLevels             = 1;
            lutDesc.Format                = DXGI_FORMAT_R32G32B32A32_FLOAT;
            lutDesc.Usage                 = D3D11_USAGE_DEFAULT;
            lutDesc.BindFlags             = D3D11_BIND_SHADER_RESOURCE;
            D3D11_SUBRESOURCE_DATA lutInit = {};
            lutInit.pSysMem               = lutData.data();
            lutInit.SysMemPitch           = LUT_SIZE * sizeof(XMFLOAT4);
            lutInit.SysMemSlicePitch      = LUT_SIZE * LUT_SIZE * sizeof(XMFLOAT4);
            if (FAILED(m_graphics->getDevice()->CreateTexture3D(&lutDesc, &lutInit, &m_lutTex)))
                throw std::runtime_error("Failed to create LUT 3D texture");
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format              = DXGI_FORMAT_R32G32B32A32_FLOAT;
            srvDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE3D;
            srvDesc.Texture3D.MipLevels = 1;
            if (FAILED(m_graphics->getDevice()->CreateShaderResourceView(m_lutTex.Get(), &srvDesc, &m_lutSRV)))
                throw std::runtime_error("Failed to create LUT SRV");
        }
        {
            renderer::SamplerDesc lutSampDesc;
            lutSampDesc.filter = renderer::FilterMode::Trilinear;
            lutSampDesc.wrapU  = renderer::WrapMode::Clamp;
            lutSampDesc.wrapV  = renderer::WrapMode::Clamp;
            m_lutSampler = std::make_unique<renderer::SamplerState>(
                m_graphics->getDevice(), m_graphics->getDeviceContext(), lutSampDesc);
        }
        {
            renderer::RenderTargetDesc cgDesc;
            cgDesc.width    = w;
            cgDesc.height   = h;
            cgDesc.format   = renderer::RenderTargetFormat::RGBA8_UNORM;
            cgDesc.hasDepth = false;
            m_colorGradingRT = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), cgDesc);
        }
        m_colorGradingPass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), L"shaders/ColorGrading.ps.hlsl");
        m_colorGradingData.lutStrength = 1.0f;
        m_colorGradingData.lutEnabled  = 1;
        m_colorGradingData._pad0       = 0.0f;
        m_colorGradingData._pad1       = 0.0f;
        m_colorGradingCB = std::make_unique<renderer::ConstantBuffer<renderer::ColorGradingData>>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        LOG_DEBUG("Creating Vignette pass...");
        {
            renderer::RenderTargetDesc vigDesc;
            vigDesc.width    = w;
            vigDesc.height   = h;
            vigDesc.format   = renderer::RenderTargetFormat::RGBA8_UNORM;
            vigDesc.hasDepth = false;
            m_vignetteRT = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), vigDesc);
        }
        m_vignettePass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), L"shaders/Vignette.ps.hlsl");
        m_vignetteData.innerRadius        = 0.3f;
        m_vignetteData.outerRadius        = 0.75f;
        m_vignetteData.intensity          = 0.6f;
        m_vignetteData.vignetteEnabled    = 1;
        m_vignetteData.aberrationStrength = 0.003f;
        m_vignetteData.aberrationEnabled  = 1;
        m_vignetteData._pad0              = 0.0f;
        m_vignetteData._pad1              = 0.0f;
        m_vignetteCB = std::make_unique<renderer::ConstantBuffer<renderer::VignetteData>>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());

        m_lightBuffer.ambientColor = { 0.3f, 0.3f, 0.35f, 1.0f };
        m_lightBuffer.numLights    = 0;

        LOG_INFO("Render pipeline initialized");
    }

    void Application::renderFrame(float deltaTime) {
        using namespace DirectX;

        auto* ctx = m_graphics->getDeviceContext();

        XMMATRIX view       = m_camera->getViewMatrix();
        XMMATRIX projection = m_camera->getProjectionMatrix();
        XMMATRIX viewProj   = XMMatrixMultiply(view, projection);
        XMMATRIX invViewProj = XMMatrixInverse(nullptr, viewProj);

        m_frustum.buildFromViewProjection(XMMatrixMultiply(view, projection));

        m_lightBuffer.viewPos = m_camera->getPosition();
        m_lightCB->update(m_lightBuffer);

        m_scene->update(deltaTime);

        m_graphics->setDepthWriteEnabled(true);
        m_graphics->setDepthFunc(renderer::DepthFunc::Less);
        m_graphics->setCullMode(renderer::CullMode::Back);
        m_graphics->setFillMode(renderer::FillMode::Solid);
        m_graphics->setBlendMode(renderer::BlendMode::Opaque);

        const XMFLOAT3 sceneCenter    = { 0.0f, 0.0f, 0.0f };
        const XMFLOAT3 directionalDir = { 1.0f, -1.0f, 1.0f };

        m_shadowPass->begin(m_graphics.get(), directionalDir, sceneCenter);
        m_shadowShader->bind(ctx);
        m_shadowPass->getShadowCB()->bindVS(3);
        m_scene->renderDepthOnly(m_transformCB.get(), m_shadowPass->getShadowData().lightSpaceMatrix);
        int vpW = m_window ? m_window->getWidth()  : static_cast<int>(m_sceneRT->getWidth());
        int vpH = m_window ? m_window->getHeight() : static_cast<int>(m_sceneRT->getHeight());
        m_shadowPass->end(m_graphics.get(), vpW, vpH);

        m_graphics->setDepthWriteEnabled(true);
        m_graphics->setDepthFunc(renderer::DepthFunc::Less);
        m_graphics->setCullMode(renderer::CullMode::Back);
        m_graphics->setFillMode(renderer::FillMode::Solid);
        m_graphics->setBlendMode(renderer::BlendMode::Opaque);

        {
            renderer::RenderTarget* activeRT    = m_msaaEnabled ? m_msaaRT.get() : m_sceneRT.get();
            ID3D11RenderTargetView* rtvs[2]     = { activeRT->getRTV(), m_normalsRT->getRTV() };
            activeRT->unbindSRV(ctx, 0);
            activeRT->unbindDepthSRV(ctx, 0);
            m_normalsRT->unbindSRV(ctx, 1);
            ctx->OMSetRenderTargets(2, rtvs, activeRT->getDSV());
            float color[4] = { 0.08f, 0.08f, 0.12f, 1.0f };
            float black[4] = { 0.0f,  0.0f,  0.0f,  0.0f };
            ctx->ClearRenderTargetView(rtvs[0], color);
            ctx->ClearRenderTargetView(rtvs[1], black);
            ctx->ClearDepthStencilView(activeRT->getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            D3D11_VIEWPORT vp = {};
            vp.Width          = static_cast<float>(activeRT->getWidth());
            vp.Height         = static_cast<float>(activeRT->getHeight());
            vp.MinDepth       = 0.0f;
            vp.MaxDepth       = 1.0f;
            ctx->RSSetViewports(1, &vp);
        }

        if (m_skyboxTexture) {
            m_graphics->setDepthWriteEnabled(false);
            m_graphics->setDepthFunc(renderer::DepthFunc::LessEqual);
            m_graphics->setCullMode(renderer::CullMode::None);
            m_graphics->setDepthClipEnabled(false);

            XMMATRIX viewNoTranslation   = view;
            viewNoTranslation.r[3]       = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
            XMMATRIX vp                  = XMMatrixMultiply(viewNoTranslation, projection);
            XMMATRIX invVP               = XMMatrixInverse(nullptr, vp);

            renderer::SkyboxData sd;
            sd.invViewProj = XMMatrixTranspose(invVP);
            sd.dummy       = XMMatrixIdentity();
            m_skyboxCB->update(sd);

            m_skyboxShader->bind(ctx);
            m_skyboxCB->bindVS(0);
            m_skyboxCB->bindPS(0);
            m_skyboxTexture->bindPS(0);
            m_skyboxSampler->bindPS(0);
            ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
            ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
            ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            ctx->Draw(3, 0);

            m_graphics->setCullMode(renderer::CullMode::Back);
            m_graphics->setDepthClipEnabled(true);
            m_graphics->setDepthWriteEnabled(true);
            m_graphics->setDepthFunc(renderer::DepthFunc::Less);
        }

        m_graphics->setBlendMode(renderer::BlendMode::Opaque);

        {
            renderer::RenderTarget* activeRT    = m_msaaEnabled ? m_msaaRT.get() : m_sceneRT.get();
            ID3D11RenderTargetView* rtvs[2]     = { activeRT->getRTV(), m_normalsRT->getRTV() };
            activeRT->unbindSRV(ctx, 0);
            activeRT->unbindDepthSRV(ctx, 0);
            m_normalsRT->unbindSRV(ctx, 1);
            ctx->OMSetRenderTargets(2, rtvs, activeRT->getDSV());
            D3D11_VIEWPORT vp = {};
            vp.Width          = static_cast<float>(activeRT->getWidth());
            vp.Height         = static_cast<float>(activeRT->getHeight());
            vp.MinDepth       = 0.0f;
            vp.MaxDepth       = 1.0f;
            ctx->RSSetViewports(1, &vp);
        }

        m_shadowPass->getShadowCB()->bindVS(3);
        m_shadowPass->getShadowCB()->bindPS(3);
        m_shadowMap->bindAsResource(ctx, 3);
        m_shadowSampler->bindPS(ctx, 1);

        m_occlusionQuery->begin(ctx);
        m_scene->render();
        m_occlusionQuery->end(ctx);

        m_shadowMap->unbindAsResource(ctx, 3);

        if (m_msaaEnabled)
            m_msaaRT->resolveInto(ctx, m_sceneRT.get());

        if (m_ssaoEnabled) {
            m_ssaoCB->update(m_ssaoData);
            m_ssaoCB->bindPS(0);
            m_ssaoClampSampler->bindPS(0);
            m_ssaoNoiseSampler->bindPS(1);
            m_sceneRT->bindDepthSRV(ctx, 0);
            m_normalsRT->bindSRV(ctx, 1);
            { ID3D11ShaderResourceView* s = m_ssaoNoiseSRV.Get(); ctx->PSSetShaderResources(2, 1, &s); }
            m_ssaoPass->render(m_ssaoRT.get(), m_ssaoRT.get(), m_graphics.get());
            m_sceneRT->unbindDepthSRV(ctx, 0);
            m_normalsRT->unbindSRV(ctx, 1);
            { ID3D11ShaderResourceView* n = nullptr; ctx->PSSetShaderResources(2, 1, &n); }
            m_ssaoNoiseSampler->unbindPS(1);

            m_ssaoBlurCB->update(m_ssaoBlurData);
            m_ssaoBlurCB->bindPS(0);
            m_ssaoClampSampler->bindPS(0);
            m_ssaoRT->bindSRV(ctx, 0);
            m_sceneRT->bindDepthSRV(ctx, 1);
            m_ssaoBlurPass->render(m_ssaoRT.get(), m_ssaoBlurRT.get(), m_graphics.get());
            m_ssaoRT->unbindSRV(ctx, 0);
            m_sceneRT->unbindDepthSRV(ctx, 1);
            m_ssaoClampSampler->unbindPS(0);
        }

        if (m_bloomEnabled) {
            m_bloomCB->update(m_bloomData);
            m_bloomCB->bindPS(0);
            m_bloomBrightPass->render(m_sceneRT.get(), m_bloomBrightRT.get(), m_graphics.get());

            m_blurData.horizontal = 1;
            m_blurCB->update(m_blurData);
            m_blurCB->bindPS(0);
            m_bloomBlurHPass->render(m_bloomBrightRT.get(), m_bloomBlurHRT.get(), m_graphics.get());

            m_blurData.horizontal = 0;
            m_blurCB->update(m_blurData);
            m_blurCB->bindPS(0);
            m_bloomBlurVPass->render(m_bloomBlurHRT.get(), m_bloomBlurRT.get(), m_graphics.get());

            m_bloomData.ssaoEnabled = m_ssaoEnabled ? 1.0f : 0.0f;
            m_bloomCB->update(m_bloomData);
            m_bloomCB->bindPS(0);
            renderer::RenderTarget* ssaoInput = m_ssaoEnabled ? m_ssaoBlurRT.get() : m_ssaoRT.get();
            m_bloomCompositePass->renderMulti(
                { m_sceneRT.get(), m_bloomBlurRT.get(), ssaoInput },
                m_bloomCompositeRT.get(), m_graphics.get());
        }

        renderer::RenderTarget* hdrInput = m_bloomEnabled ? m_bloomCompositeRT.get() : m_sceneRT.get();

        {
            m_motionBlurData.prevViewProj = XMMatrixTranspose(m_prevViewProj);
            m_motionBlurData.invViewProj  = XMMatrixTranspose(invViewProj);
            m_motionBlurData.enabled      = m_motionBlurEnabled ? 1 : 0;
            m_motionBlurCB->update(m_motionBlurData);
            m_motionBlurCB->bindPS(0);
            m_sceneRT->bindDepthSRV(ctx, 1);
            m_motionBlurPass->render(hdrInput, m_motionBlurRT.get(), m_graphics.get());
            m_sceneRT->unbindDepthSRV(ctx, 1);
            hdrInput = m_motionBlurRT.get();
        }
        {
            m_dofData.enabled = m_dofEnabled ? 1 : 0;
            m_dofCB->update(m_dofData);
            m_dofCB->bindPS(0);
            m_sceneRT->bindDepthSRV(ctx, 1);
            m_dofPass->render(hdrInput, m_dofRT.get(), m_graphics.get());
            m_sceneRT->unbindDepthSRV(ctx, 1);
            hdrInput = m_dofRT.get();
        }

        m_postProcessCB->update(m_postProcessData);
        m_postProcessCB->bindPS(0);
        m_blitPass->render(hdrInput, m_ldrRT.get(), m_graphics.get());

        renderer::RenderTarget* ldrInput = m_ldrRT.get();

        {
            m_colorGradingData.lutEnabled = m_colorGradingEnabled ? 1 : 0;
            m_colorGradingCB->update(m_colorGradingData);
            m_colorGradingCB->bindPS(0);
            { ID3D11ShaderResourceView* s = m_lutSRV.Get(); ctx->PSSetShaderResources(1, 1, &s); }
            m_lutSampler->bindPS(0);
            m_colorGradingPass->render(ldrInput, m_colorGradingRT.get(), m_graphics.get());
            { ID3D11ShaderResourceView* n = nullptr; ctx->PSSetShaderResources(1, 1, &n); }
            m_lutSampler->unbindPS(0);
            ldrInput = m_colorGradingRT.get();
        }
        {
            m_vignetteData.vignetteEnabled   = m_vignetteEnabled   ? 1 : 0;
            m_vignetteData.aberrationEnabled = m_aberrationEnabled ? 1 : 0;
            m_vignetteCB->update(m_vignetteData);
            m_vignetteCB->bindPS(0);
            m_vignettePass->render(ldrInput, m_vignetteRT.get(), m_graphics.get());
            ldrInput = m_vignetteRT.get();
        }

        m_fxaaCB->update(m_fxaaData);
        m_fxaaCB->bindPS(0);
        m_fxaaPass->renderToBackBuffer(ldrInput, m_graphics.get());

        m_prevViewProj = viewProj;
    }

    int Application::run() {
        using Clock    = std::chrono::high_resolution_clock;
        using Duration = std::chrono::duration<float>;

        LOG_INFO("Entering main loop");

        auto lastTime = Clock::now();

        try {
            while (m_window->processMessages()) {
                m_graphics->beginFrame(0.0f, 0.0f, 0.0f);

                auto  now       = Clock::now();
                float deltaTime = Duration(now - lastTime).count();
                lastTime        = now;

                auto& input = InputSystem::get();

                if (input.isKeyPressed(Key::Escape)) {
                    if (input.isMouseCaptured()) input.setMouseCaptured(false);
                    else break;
                }

                m_physicsWorld->simulate(deltaTime);

                for (const auto& ev : m_physicsWorld->getContactEvents()) {
                    auto dispatch = [&](uint64_t selfId, uint64_t otherId) {
                        Entity* e = m_scene->findById(selfId);
                        if (!e) return;
                        if (auto* rb = e->getComponent<physics::RigidBody>()) {
                            using T = physics::ContactEvent::Type;
                            if      (ev.type == T::Enter) rb->onCollisionEnter(otherId);
                            else if (ev.type == T::Stay)  rb->onCollisionStay(otherId);
                            else if (ev.type == T::Exit)  rb->onCollisionExit(otherId);
                        }
                    };
                    dispatch(ev.entityA, ev.entityB);
                    dispatch(ev.entityB, ev.entityA);
                }

                m_assetManager->flushPendingUploads();
                m_audioEngine->update(deltaTime);

                {
                    std::vector<std::string> shaderReloads;
                    std::vector<std::string> textureReloads;
                    {
                        std::unique_lock lock(m_reloadMutex);
                        shaderReloads  = std::move(m_pendingShaderReloads);
                        textureReloads = std::move(m_pendingTextureReloads);
                    }
                    for (const auto& path : shaderReloads) {
                        auto ws = std::wstring(path.begin(), path.end());
                        for (auto* s : { m_shadowShader.get(), m_skyboxShader.get() }) {
                            if (s && (s->getPsPath() == ws || s->getVsPath() == ws))
                                s->tryReload(m_graphics->getDevice());
                        }
                    }
                    (void)textureReloads;
                }

                onUpdate(deltaTime);
                renderFrame(deltaTime);
                onRender();

                m_graphics->endFrame();
                input.endFrame();
            }
        } catch (const std::exception& e) {
            LOG_FATAL(std::string("Exception in main loop: ") + e.what());
            MessageBoxA(nullptr, e.what(), "Runtime Error", MB_OK | MB_ICONERROR);
            return -1;
        }

        LOG_INFO("Main loop exited cleanly");
        return 0;
    }

}