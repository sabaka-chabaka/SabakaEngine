#include "core/Application.h"
#include "core/Logger.h"
#include "platform/Input.h"
#include <DirectXMath.h>
#include <chrono>
#include <cmath>
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
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

        Logger::get().openLogFile("engine.log");
        Logger::get().setMinLevel(LogLevel::Debug);

        LOG_INFO("Initializing SabakaEngine");

        platform::WindowDesc wDesc;
        wDesc.title = L"SabakaEngine v0.1";
        wDesc.width = 1280;
        wDesc.height = 720;

        LOG_DEBUG("Creating window...");
        m_window = std::make_unique<platform::Window>(wDesc);
        LOG_INFO("Window created: 1280x720");

        LOG_DEBUG("Initializing input system...");
        InputSystem::get().initialize(m_window->getNativeHandle());

        renderer::GraphicsDeviceDesc gDesc;
        gDesc.hwnd = m_window->getNativeHandle();
        gDesc.width = m_window->getWidth();
        gDesc.height = m_window->getHeight();
        gDesc.vsync = true;

        LOG_DEBUG("Initializing GraphicsDevice...");
        m_graphics = std::make_unique<renderer::GraphicsDevice>(gDesc);
        LOG_INFO("DirectX 11 device initialized");

        LOG_DEBUG("Creating base cube mesh...");
        m_mesh = std::make_unique<renderer::Mesh>(
            renderer::Mesh::createCube(
                m_graphics->getDevice(),
                m_graphics->getDeviceContext()
            )
        );

        std::vector<renderer::InputElementDesc> cubeLayout = {
            {"POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0},
            {"COLOR",    DXGI_FORMAT_R32G32B32_FLOAT, 12},
            {"TEXCOORD", DXGI_FORMAT_R32G32_FLOAT,    24},
            {"NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT, 32},
        };

        LOG_DEBUG("Compiling main shader...");
        m_shader = std::make_unique<renderer::Shader>(
            m_graphics->getDevice(),
            L"shaders/Basic.vs.hlsl",
            L"shaders/Basic.ps.hlsl",
            cubeLayout
        );

        LOG_DEBUG("Creating constant buffers...");
        m_transformCB = std::make_unique<renderer::ConstantBuffer<renderer::TransformData> >(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext()
        );

        m_materialCB = std::make_unique<renderer::ConstantBuffer<renderer::MaterialData> >(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext()
        );

        m_lightCB = std::make_unique<renderer::ConstantBuffer<renderer::LightData>>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext()
        );

        std::string exeDir = getExeDir();

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

        renderer::SamplerDesc sampDesc;
        sampDesc.filter = renderer::FilterMode::Trilinear;
        sampDesc.wrapU = renderer::WrapMode::Repeat;
        sampDesc.wrapV = renderer::WrapMode::Repeat;

        LOG_DEBUG("Creating main sampler...");
        m_sampler = std::make_unique<renderer::SamplerState>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext(),
            sampDesc
        );

        LOG_DEBUG("Initializing skybox...");
        LOG_DEBUG("Creating skybox...");
        m_skyboxMesh = std::make_unique<renderer::SkyboxMesh>(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext()
        );

        std::vector<renderer::InputElementDesc> skyboxLayout = {};

        LOG_DEBUG("Compiling skybox shader...");
        m_skyboxShader = std::make_unique<renderer::Shader>(
            m_graphics->getDevice(),
            L"shaders/Skybox.vs.hlsl",
            L"shaders/Skybox.ps.hlsl",
            skyboxLayout
        );

        m_skyboxCB = std::make_unique<renderer::ConstantBuffer<renderer::SkyboxData> >(
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
        camDesc.position = {0.0f, 1.5f, -4.0f};
        camDesc.target = {0.0f, 0.0f, 0.0f};
        camDesc.up = {0.0f, 1.0f, 0.0f};
        camDesc.fovY = XM_PIDIV4;
        camDesc.nearZ = 0.1f;
        camDesc.farZ = 1000.0f;

        m_camera = std::make_unique<renderer::Camera>(camDesc);
        m_camera->setAspectRatio(
            static_cast<float>(m_window->getWidth()) /
            static_cast<float>(m_window->getHeight())
        );

        m_window->setResizeCallback([this](int w, int h) {
            LOG_DEBUG("Window resized: " + std::to_string(w) + "x" + std::to_string(h));
            m_graphics->onResize(w, h);
            if (m_camera && h > 0) {
                m_camera->setAspectRatio(static_cast<float>(w) / static_cast<float>(h));
            }
        });

        LOG_INFO("Scene resources ready");
    }

    int Application::run() {
        using Clock = std::chrono::high_resolution_clock;
        using Duration = std::chrono::duration<float>;

        LOG_INFO("Entering main loop");

        auto lastTime = Clock::now();
        float totalTime = 0.0f;

        renderer::MaterialData material;
        material.specularIntensity = 0.6f;
        material.specularPower = 32.0f;
        material.uvScale = {1.0f, 1.0f};
        material.uvOffset = {0.0f, 0.0f};

        renderer::LightData light;
        light.ambientColor    = { 0.3f, 0.3f, 0.35f, 1.0f };
        light.lightDirection  = { 1.0f, -1.0f, 1.0f };
        light.lightColor      = { 1.0f, 1.0f, 0.9f, 1.0f };

        light.pointLightPos   = { 2.0f, 1.5f, 0.0f, 1.0f };
        light.pointLightColor = { 1.0f, 0.45f, 0.1f, 1.0f };
        light.attConstant     = 1.0f;
        light.attLinear       = 0.22f;
        light.attQuadratic    = 0.20f;
        light.pointEnabled    = 1.0f;

        try {
            while (m_window->processMessages()) {
                auto now = Clock::now();
                float deltaTime = Duration(now - lastTime).count();
                lastTime = now;
                totalTime += deltaTime;

                auto &input = InputSystem::get();

                if (input.isKeyPressed(Key::Escape)) {
                    if (input.isMouseCaptured()) {
                        input.setMouseCaptured(false);
                    } else {
                        break;
                    }
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
                    light.pointEnabled = (light.pointEnabled > 0.5f) ? 0.0f : 1.0f;
                    LOG_INFO(light.pointEnabled > 0.5f ? "Point light ON" : "Point light OFF");
                }

                onUpdate(deltaTime);

                XMMATRIX view = m_camera->getViewMatrix();
                XMMATRIX projection = m_camera->getProjectionMatrix();

                m_graphics->beginFrame(0.08f, 0.08f, 0.12f);

                m_graphics->setDepthWriteEnabled(false);
                m_graphics->setDepthFunc(renderer::DepthFunc::LessEqual);
                m_graphics->setCullMode(renderer::CullMode::None);
                m_graphics->setDepthClipEnabled(false);

                {
                    XMMATRIX viewNoTranslation = view;
                    viewNoTranslation.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

                    XMMATRIX vp = XMMatrixMultiply(viewNoTranslation, projection);
                    XMMATRIX invVP = XMMatrixInverse(nullptr, vp);

                    renderer::SkyboxData sd;
                    sd.invViewProj = XMMatrixTranspose(invVP);
                    sd.dummy = XMMatrixIdentity();

                    m_skyboxCB->update(sd);

                    m_skyboxShader->bind(m_graphics->getDeviceContext());
                    m_skyboxCB->bindVS(0);
                    m_skyboxCB->bindPS(0);
                    m_skyboxTexture->bindPS(0);
                    m_skyboxSampler->bindPS(0);

                    auto* ctx = m_graphics->getDeviceContext();
                    ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
                    ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
                    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    ctx->Draw(3, 0);
                }

                m_graphics->setBlendMode(renderer::BlendMode::Opaque);
                m_graphics->setDepthWriteEnabled(true);
                m_graphics->setDepthFunc(renderer::DepthFunc::Less);
                m_graphics->setCullMode(renderer::CullMode::Back);
                m_graphics->setDepthClipEnabled(true);

                {
                    XMMATRIX model = XMMatrixRotationRollPitchYaw(
                        totalTime * 0.4f,
                        totalTime * 0.8f,
                        totalTime * 0.2f
                    );

                    XMMATRIX normalMatrix = XMMatrixTranspose(
                        XMMatrixInverse(nullptr, model)
                    );

                    renderer::TransformData td;
                    td.model        = XMMatrixTranspose(model);
                    td.view         = XMMatrixTranspose(view);
                    td.projection   = XMMatrixTranspose(projection);
                    td.normalMatrix = XMMatrixTranspose(normalMatrix);

                    m_transformCB->update(td);
                    m_transformCB->bindVS(0);

                    m_materialCB->update(material);
                    m_materialCB->bindPS(1);

                    float ptOrbitRadius = 2.5f;
                    light.pointLightPos = {
                        ptOrbitRadius * cosf(totalTime),
                        1.5f,
                        ptOrbitRadius * sinf(totalTime),
                        1.0f
                    };

                    light.viewPos = m_camera->getPosition();
                    m_lightCB->update(light);
                    m_lightCB->bindPS(2);

                    m_shader->bind(m_graphics->getDeviceContext());

                    m_diffuseTexture->bindPS(0);
                    m_specularTexture->bindPS(1);
                    m_sampler->bindPS(0);
                    m_mesh->draw();
                }

                onRender();
                m_graphics->endFrame();

                input.endFrame();
            }
        } catch (const std::exception &e) {
            LOG_FATAL(std::string("Exception in main loop: ") + e.what());
            MessageBoxA(nullptr, e.what(), "Runtime Error", MB_OK | MB_ICONERROR);
            return -1;
        }

        LOG_INFO("Main loop exited cleanly");
        return 0;
    }
}