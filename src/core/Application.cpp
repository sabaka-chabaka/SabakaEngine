#include "core/Application.h"
#include "core/Logger.h"
#include "math/AABB.h"
#include "platform/Input.h"
#include <DirectXMath.h>
#include <chrono>
#include <cmath>
#include <filesystem>

#include "io/ObjLoader.h"

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
        wDesc.title = L"SabakaEngine v0.8";
        wDesc.width = 1280;
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

        LOG_DEBUG("Creating base cube mesh...");
        try {
            m_mesh = std::make_unique<renderer::Mesh>(
                io::ObjLoader::load(m_graphics->getDevice(), m_graphics->getDeviceContext(), exeDir + "/models/guy.obj")
            );
        }
        catch (const std::exception& e) {
            MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
            return;
        }

        std::vector<renderer::InputElementDesc> cubeLayout = {
            {"POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0},
            {"COLOR",    DXGI_FORMAT_R32G32B32_FLOAT, 12},
            {"TEXCOORD", DXGI_FORMAT_R32G32_FLOAT,    24},
            {"NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT, 32},
            {"TANGENT",  DXGI_FORMAT_R32G32B32_FLOAT, 44},
        };

        LOG_DEBUG("Compiling main shader...");
        m_shader = std::make_unique<renderer::Shader>(
            m_graphics->getDevice(),
            L"shaders/Basic.vs.hlsl",
            L"shaders/Basic.ps.hlsl",
            cubeLayout
        );

        std::vector<renderer::InputElementDesc> shadowLayout = {
            {"POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0},
        };

        LOG_DEBUG("Compiling shadow shader...");
        m_shadowShader = std::make_unique<renderer::Shader>(
            m_graphics->getDevice(),
            L"shaders/Shadow.vs.hlsl",
            L"shaders/Shadow.ps.hlsl",
            shadowLayout
        );

        LOG_DEBUG("Creating constant buffers...");
        m_transformCB = std::make_unique<renderer::ConstantBuffer<renderer::TransformData>>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext()
        );

        m_lightCB = std::make_unique<renderer::ConstantBuffer<renderer::LightBuffer>>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext()
        );

        renderer::TextureDesc texDesc;
        texDesc.generateMips = true;

        LOG_DEBUG("Loading textures...");
        m_diffuseTexture = std::make_unique<renderer::Texture2D>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext(),
            exeDir + "/textures/cube_diffuse.png",
            texDesc
        );

        m_specularTexture = std::make_unique<renderer::Texture2D>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext(),
            exeDir + "/textures/cube_specular.png",
            texDesc
        );

        LOG_DEBUG("Loading normal map...");
        m_normalMap = std::make_unique<renderer::Texture2D>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext(),
            exeDir + "/textures/cube_normal.png",
            texDesc
        );

        renderer::SamplerDesc sampDesc;
        sampDesc.filter = renderer::FilterMode::Trilinear;
        sampDesc.wrapU  = renderer::WrapMode::Repeat;
        sampDesc.wrapV  = renderer::WrapMode::Repeat;

        LOG_DEBUG("Creating main sampler...");
        m_sampler = std::make_unique<renderer::SamplerState>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext(),
            sampDesc
        );

        LOG_DEBUG("Creating skybox...");
        m_skyboxMesh = std::make_unique<renderer::SkyboxMesh>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext()
        );

        LOG_DEBUG("Compiling skybox shader...");
        m_skyboxShader = std::make_unique<renderer::Shader>(
            m_graphics->getDevice(),
            L"shaders/Skybox.vs.hlsl",
            L"shaders/Skybox.ps.hlsl",
            std::vector<renderer::InputElementDesc>{}
        );

        m_skyboxCB = std::make_unique<renderer::ConstantBuffer<renderer::SkyboxData>>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext()
        );

        std::array<std::string, 6> skyboxFaces = {
            exeDir + "/textures/skybox/right.png",
            exeDir + "/textures/skybox/left.png",
            exeDir + "/textures/skybox/top.png",
            exeDir + "/textures/skybox/bottom.png",
            exeDir + "/textures/skybox/front.png",
            exeDir + "/textures/skybox/back.png",
        };

        LOG_DEBUG("Loading skybox textures...");
        m_skyboxTexture = std::make_unique<renderer::CubemapTexture>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext(),
            skyboxFaces
        );

        renderer::SamplerDesc skyboxSampDesc;
        skyboxSampDesc.filter = renderer::FilterMode::Trilinear;
        skyboxSampDesc.wrapU  = renderer::WrapMode::Clamp;
        skyboxSampDesc.wrapV  = renderer::WrapMode::Clamp;

        LOG_DEBUG("Creating skybox sampler...");
        m_skyboxSampler = std::make_unique<renderer::SamplerState>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext(),
            skyboxSampDesc
        );

        renderer::CameraDesc camDesc;
        camDesc.position = { 0.0f, 1.5f, -15.0f };
        camDesc.target   = { 0.0f, 0.0f,  0.0f };
        camDesc.up       = { 0.0f, 1.0f,  0.0f };
        camDesc.fovY     = XM_PIDIV4;
        camDesc.nearZ    = 0.1f;
        camDesc.farZ     = 1000.0f;

        m_camera = std::make_unique<renderer::Camera>(camDesc);
        m_camera->setAspectRatio(
            static_cast<float>(m_window->getWidth()) /
            static_cast<float>(m_window->getHeight())
        );

        m_window->setResizeCallback([this](int w, int h) {
            LOG_DEBUG("Window resized: " + std::to_string(w) + "x" + std::to_string(h));
            m_graphics->onResize(w, h);
            if (m_camera && h > 0)
                m_camera->setAspectRatio(static_cast<float>(w) / static_cast<float>(h));
            if (m_sceneRT && w > 0 && h > 0)
                m_sceneRT->resize(m_graphics->getDevice(),
                    static_cast<uint32_t>(w), static_cast<uint32_t>(h));
        });

        if (m_window->getWidth() > 0 && m_window->getHeight() > 0) {
            renderer::RenderTargetDesc rtDesc;
            rtDesc.width    = static_cast<uint32_t>(m_window->getWidth());
            rtDesc.height   = static_cast<uint32_t>(m_window->getHeight());
            rtDesc.format   = renderer::RenderTargetFormat::RGBA16_FLOAT;
            rtDesc.hasDepth = true;
            m_sceneRT = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), rtDesc);
        }

        LOG_DEBUG("Creating depth pre-pass...");
        m_depthPrePass = std::make_unique<renderer::DepthPrePass>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext()
        );

        LOG_DEBUG("Creating occlusion query...");
        m_occlusionQuery = std::make_unique<renderer::OcclusionQuery>(
            m_graphics->getDevice()
        );

        LOG_DEBUG("Creating shadow map...");
        renderer::ShadowMapDesc smDesc;
        smDesc.width  = 2048;
        smDesc.height = 2048;
        m_shadowMap = std::make_unique<renderer::ShadowMap>(m_graphics->getDevice(), smDesc);

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
            m_graphics->getDevice(),
            m_graphics->getDeviceContext(),
            m_shadowMap.get(),
            spDesc
        );

        LOG_DEBUG("Creating shadow sampler...");
        m_shadowSampler = std::make_unique<renderer::ShadowSampler>(m_graphics->getDevice());

        LOG_DEBUG("Creating scene render target...");
        if (!m_sceneRT) {
            renderer::RenderTargetDesc rtDesc;
            rtDesc.width    = static_cast<uint32_t>(m_window->getWidth());
            rtDesc.height   = static_cast<uint32_t>(m_window->getHeight());
            rtDesc.format   = renderer::RenderTargetFormat::RGBA16_FLOAT;
            rtDesc.hasDepth = true;
            m_sceneRT = std::make_unique<renderer::RenderTarget>(m_graphics->getDevice(), rtDesc);
        }

        LOG_DEBUG("Creating blit pass...");
        m_blitPass = std::make_unique<renderer::PostProcessPass>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext(),
            L"shaders/Blit.ps.hlsl"
        );

        LOG_DEBUG("Creating scene...");
        m_scene     = std::make_unique<Scene>();
        m_hierarchy = std::make_unique<SceneHierarchy>();

        m_cubeEntity = m_scene->createEntity("Cube");
        m_cubeEntity->addComponent<Transform>();

        LOG_DEBUG("Creating material...");
        m_cubeMaterial = std::make_unique<renderer::Material>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext()
        );
        m_cubeMaterial->setShader(m_shader.get());
        m_cubeMaterial->setDiffuseTexture(m_diffuseTexture.get());
        m_cubeMaterial->setSpecularTexture(m_specularTexture.get());
        m_cubeMaterial->setNormalMap(m_normalMap.get());
        m_cubeMaterial->setSampler(m_sampler.get());

        renderer::MaterialData matData;
        matData.specularIntensity = 0.6f;
        matData.specularPower     = 32.0f;
        matData.uvScale           = { 1.0f, 1.0f };
        matData.uvOffset          = { 0.0f, 0.0f };
        matData.useNormalMap      = 1.0f;
        m_cubeMaterial->setData(matData);

        auto* mr = m_cubeEntity->addComponent<MeshRenderer>();
        mr->setMesh(m_mesh.get());
        mr->setMaterial(m_cubeMaterial.get());
        mr->setTransformCB(m_transformCB.get());
        mr->setLightCB(m_lightCB.get());
        mr->setCamera(m_camera.get());
        mr->setFrustum(&m_frustum);

        auto* bb = m_cubeEntity->addComponent<BoundingBoxComponent>();
        math::AABB cubeAABB;
        cubeAABB.min = { -0.5f, -0.5f, -0.5f };
        cubeAABB.max = {  0.5f,  0.5f,  0.5f };
        bb->setLocalAABB(cubeAABB);

        LOG_INFO("Scene resources ready");
    }

    int Application::run() {
        using Clock    = std::chrono::high_resolution_clock;
        using Duration = std::chrono::duration<float>;

        LOG_INFO("Entering main loop");

        auto lastTime = Clock::now();

        renderer::LightBuffer lightBuf;
        lightBuf.ambientColor = { 0.3f, 0.3f, 0.35f, 1.0f };

        lightBuf.lights[0] = renderer::makeDirectionalLight(
            { 1.0f, -1.0f, 1.0f },
            { 1.0f,  1.0f, 0.9f },
            0.8f
        );

        lightBuf.lights[1] = renderer::makePointLight(
            { 2.0f, 1.5f, 0.0f },
            { 1.0f, 0.45f, 0.1f },
            1.5f,
            8.0f
        );

        lightBuf.lights[2] = renderer::makeSpotLight(
            {  0.0f,  4.0f, 0.0f },
            {  0.0f, -1.0f, 0.0f },
            {  0.2f,  0.6f, 1.0f },
            10.0f,
            22.0f,
            2.0f,
            12.0f
        );

        lightBuf.numLights = 3;

        float orbitAngle = 0.0f;

        const XMFLOAT3 sceneCenter  = { 0.0f, 0.0f, 0.0f };
        const XMFLOAT3 directionalDir = { 1.0f, -1.0f, 1.0f };

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

                if (input.isKeyPressed(Key::F1)) {
                    m_graphics->setFillMode(renderer::FillMode::Wireframe);
                    LOG_INFO("Wireframe ON");
                }
                if (input.isKeyPressed(Key::F2)) {
                    m_graphics->setFillMode(renderer::FillMode::Solid);
                    LOG_INFO("Wireframe OFF");
                }
                if (input.isKeyPressed(Key::F3)) {
                    float& en = lightBuf.lights[1].params.w;
                    en = (en > 0.5f) ? 0.0f : 1.0f;
                    LOG_INFO(en > 0.5f ? "Point light ON" : "Point light OFF");
                }
                if (input.isKeyPressed(Key::F4)) {
                    float& en = lightBuf.lights[2].params.w;
                    en = (en > 0.5f) ? 0.0f : 1.0f;
                    LOG_INFO(en > 0.5f ? "Spot light ON" : "Spot light OFF");
                }
                if (input.isKeyPressed(Key::F5)) {
                    auto& matData = m_cubeMaterial->getData();
                    matData.useNormalMap = (matData.useNormalMap > 0.5f) ? 0.0f : 1.0f;
                    LOG_INFO(matData.useNormalMap > 0.5f ? "Normal map ON" : "Normal map OFF");
                }

                auto* transform = m_cubeEntity->getComponent<Transform>();
                transform->rotateEuler(
                    deltaTime * 0.4f,
                    deltaTime * 0.8f,
                    deltaTime * 0.2f
                );

                orbitAngle += deltaTime;

                const float ptOrbitRadius = 2.5f;
                lightBuf.lights[1].positionAndType = {
                    ptOrbitRadius * cosf(orbitAngle),
                    1.5f,
                    ptOrbitRadius * sinf(orbitAngle),
                    static_cast<float>(renderer::LightType::Point)
                };

                lightBuf.viewPos = m_camera->getPosition();
                m_lightCB->update(lightBuf);

                XMMATRIX view       = m_camera->getViewMatrix();
                XMMATRIX projection = m_camera->getProjectionMatrix();

                m_frustum.buildFromViewProjection(view * projection);

                m_scene->update(deltaTime);
                onUpdate(deltaTime);

                auto* ctx = m_graphics->getDeviceContext();

                m_graphics->setDepthWriteEnabled(true);
                m_graphics->setDepthFunc(renderer::DepthFunc::Less);
                m_graphics->setCullMode(renderer::CullMode::Back);
                m_graphics->setFillMode(renderer::FillMode::Solid);
                m_graphics->setBlendMode(renderer::BlendMode::Opaque);

                m_shadowPass->begin(m_graphics.get(), directionalDir, sceneCenter);
                m_shadowShader->bind(ctx);
                m_shadowPass->getShadowCB()->bindVS(3);
                m_scene->renderDepthOnly(m_transformCB.get(), m_shadowPass->getShadowData().lightSpaceMatrix);
                m_shadowPass->end(m_graphics.get(), m_window->getWidth(), m_window->getHeight());

                m_graphics->setDepthWriteEnabled(true);
                m_graphics->setDepthFunc(renderer::DepthFunc::Less);
                m_graphics->setCullMode(renderer::CullMode::Back);
                m_graphics->setFillMode(renderer::FillMode::Solid);
                m_graphics->setBlendMode(renderer::BlendMode::Opaque);

                {
                    ID3D11RenderTargetView* rtv = m_sceneRT->getRTV();
                    ctx->OMSetRenderTargets(1, &rtv, m_sceneRT->getDSV());
                    float color[4] = { 0.08f, 0.08f, 0.12f, 1.0f };
                    ctx->ClearRenderTargetView(rtv, color);
                    ctx->ClearDepthStencilView(m_sceneRT->getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
                    D3D11_VIEWPORT vp = {};
                    vp.Width    = static_cast<float>(m_sceneRT->getWidth());
                    vp.Height   = static_cast<float>(m_sceneRT->getHeight());
                    vp.MinDepth = 0.0f;
                    vp.MaxDepth = 1.0f;
                    ctx->RSSetViewports(1, &vp);
                }

                m_graphics->setDepthWriteEnabled(false);
                m_graphics->setDepthFunc(renderer::DepthFunc::LessEqual);
                m_graphics->setCullMode(renderer::CullMode::None);
                m_graphics->setDepthClipEnabled(false);

                {
                    XMMATRIX viewNoTranslation = view;
                    viewNoTranslation.r[3]     = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

                    XMMATRIX vp    = XMMatrixMultiply(viewNoTranslation, projection);
                    XMMATRIX invVP = XMMatrixInverse(nullptr, vp);

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
                }

                m_graphics->setCullMode(renderer::CullMode::Back);
                m_graphics->setDepthClipEnabled(true);
                m_graphics->setDepthWriteEnabled(true);
                m_graphics->setDepthFunc(renderer::DepthFunc::Less);

                m_graphics->setBlendMode(renderer::BlendMode::Opaque);

                m_shadowPass->getShadowCB()->bindVS(3);
                m_shadowPass->getShadowCB()->bindPS(3);
                m_shadowMap->bindAsResource(ctx, 3);
                m_shadowSampler->bindPS(ctx, 1);

                m_occlusionQuery->begin(ctx);
                m_scene->render();
                m_occlusionQuery->end(ctx);

                m_shadowMap->unbindAsResource(ctx, 3);
                m_blitPass->renderToBackBuffer(m_sceneRT.get(), m_graphics.get());

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