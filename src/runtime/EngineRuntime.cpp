#define NOMINMAX
#include "runtime/EngineRuntime.h"
#include "core/Transform.h"
#include "core/MeshRenderer.h"
#include "core/BoundingBoxComponent.h"
#include "core/Logger.h"
#include "io/ObjLoader.h"
#include "math/AABB.h"
#include "platform/Input.h"
#include "renderer/Light.h"
#include <DirectXMath.h>
#include <filesystem>
#include <windows.h>

using namespace DirectX;
using namespace engine::platform;
using namespace engine::core;

namespace engine::runtime {

    static std::string getExeDir() {
        wchar_t buf[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, buf, MAX_PATH);
        return std::filesystem::path(buf).parent_path().string();
    }

    EngineRuntime::EngineRuntime() {
        onInit();
    }

    void EngineRuntime::onInit() {
        std::string exeDir = getExeDir();

        std::vector<renderer::InputElementDesc> layout = {
            {"POSITION", DXGI_FORMAT_R32G32B32_FLOAT,    0},
            {"COLOR",    DXGI_FORMAT_R32G32B32_FLOAT,    12},
            {"TEXCOORD", DXGI_FORMAT_R32G32_FLOAT,       24},
            {"NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT,    32},
            {"TANGENT",  DXGI_FORMAT_R32G32B32A32_FLOAT, 44},
        };

        LOG_DEBUG("Loading guy.obj...");
        m_mesh = std::make_unique<renderer::Mesh>(
            io::ObjLoader::load(m_graphics->getDevice(), m_graphics->getDeviceContext(),
                exeDir + "/models/guy.obj"));

        LOG_DEBUG("Compiling Basic shader...");
        m_shader = std::make_unique<renderer::Shader>(
            m_graphics->getDevice(),
            L"shaders/Basic.vs.hlsl",
            L"shaders/Basic.ps.hlsl",
            layout);

        renderer::TextureDesc texDesc;
        texDesc.generateMips = true;

        LOG_DEBUG("Loading textures...");
        m_diffuseTexture = std::make_unique<renderer::Texture2D>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(),
            exeDir + "/textures/cube_diffuse.png", texDesc);

        m_specularTexture = std::make_unique<renderer::Texture2D>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(),
            exeDir + "/textures/cube_specular.png", texDesc);

        m_normalMap = std::make_unique<renderer::Texture2D>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(),
            exeDir + "/textures/cube_normal.png", texDesc);

        renderer::SamplerDesc sampDesc;
        sampDesc.filter = renderer::FilterMode::Trilinear;
        sampDesc.wrapU  = renderer::WrapMode::Repeat;
        sampDesc.wrapV  = renderer::WrapMode::Repeat;
        m_sampler = std::make_unique<renderer::SamplerState>(
            m_graphics->getDevice(), m_graphics->getDeviceContext(), sampDesc);

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
            m_graphics->getDevice(), m_graphics->getDeviceContext(), skyboxFaces);

        m_lightBuffer.ambientColor = { 0.3f, 0.3f, 0.35f, 1.0f };
        m_lightBuffer.lights[0]    = renderer::makeDirectionalLight(
            { 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f, 0.9f }, 0.8f);
        m_lightBuffer.lights[1]    = renderer::makePointLight(
            { 2.0f, 1.5f, 0.0f }, { 1.0f, 0.45f, 0.1f }, 1.5f, 8.0f);
        m_lightBuffer.lights[2]    = renderer::makeSpotLight(
            { 0.0f, 4.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },
            { 0.2f, 0.6f, 1.0f }, 10.0f, 22.0f, 2.0f, 12.0f);
        m_lightBuffer.numLights    = 3;

        m_cubeEntity = m_scene->createEntity("Cube");
        m_cubeEntity->addComponent<Transform>();

        LOG_DEBUG("Creating material...");
        m_cubeMaterial = std::make_unique<renderer::Material>(
            m_graphics->getDevice(), m_graphics->getDeviceContext());
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
        math::AABB aabb;
        aabb.min = { -0.5f, -0.5f, -0.5f };
        aabb.max = {  0.5f,  0.5f,  0.5f };
        bb->setLocalAABB(aabb);

        LOG_INFO("EngineRuntime scene ready");
    }

    void EngineRuntime::onUpdate(float deltaTime) {
        auto& input = InputSystem::get();

        if (input.isKeyPressed(Key::F1)) {
            m_graphics->setFillMode(renderer::FillMode::Wireframe);
            LOG_INFO("Wireframe ON");
        }
        if (input.isKeyPressed(Key::F2)) {
            m_graphics->setFillMode(renderer::FillMode::Solid);
            LOG_INFO("Wireframe OFF");
        }
        if (input.isKeyPressed(Key::F5)) {
            auto& matData    = m_cubeMaterial->getData();
            matData.useNormalMap = (matData.useNormalMap > 0.5f) ? 0.0f : 1.0f;
            LOG_INFO(matData.useNormalMap > 0.5f ? "Normal map ON" : "Normal map OFF");
        }
        if (input.isKeyPressed(Key::F6)) {
            m_sceneRT->captureToImage(m_graphics->getDeviceContext(), "sceneRT.bmp");
            m_graphics->captureToImage("backbuffer.bmp");
            LOG_INFO("Captured sceneRT.bmp and backbuffer.bmp");
        }
        if (input.isKeyPressed(Key::F7)) {
            if (m_postProcessData.tonemapMode == static_cast<int>(renderer::TonemapMode::Reinhard)) {
                m_postProcessData.tonemapMode = static_cast<int>(renderer::TonemapMode::ACES);
                LOG_INFO("Tone mapping: ACES Filmic");
            } else {
                m_postProcessData.tonemapMode = static_cast<int>(renderer::TonemapMode::Reinhard);
                LOG_INFO("Tone mapping: Reinhard");
            }
        }
        if (input.isKeyPressed(Key::F8)) {
            m_postProcessData.exposure = std::min(m_postProcessData.exposure + 0.1f, 5.0f);
            LOG_INFO("Exposure: " + std::to_string(m_postProcessData.exposure));
        }
        if (input.isKeyPressed(Key::F9)) {
            m_postProcessData.exposure = std::max(m_postProcessData.exposure - 0.1f, 0.1f);
            LOG_INFO("Exposure: " + std::to_string(m_postProcessData.exposure));
        }
        if (input.isKeyPressed(Key::F10)) {
            m_fxaaData.enabled = m_fxaaData.enabled ? 0 : 1;
            LOG_INFO(m_fxaaData.enabled ? "FXAA ON" : "FXAA OFF");
        }
        if (input.isKeyPressed(Key::F11)) {
            m_msaaEnabled = !m_msaaEnabled;
            LOG_INFO(m_msaaEnabled ? "MSAA 4x ON" : "MSAA OFF");
        }
        if (input.isKeyPressed(Key::F12)) {
            m_bloomEnabled = !m_bloomEnabled;
            LOG_INFO(m_bloomEnabled ? "Bloom ON" : "Bloom OFF");
        }
        if (input.isKeyPressed(Key::B)) {
            m_ssaoEnabled = !m_ssaoEnabled;
            LOG_INFO(m_ssaoEnabled ? "SSAO ON" : "SSAO OFF");
        }
        if (input.isKeyPressed(Key::V)) {
            m_motionBlurEnabled      = !m_motionBlurEnabled;
            m_motionBlurData.enabled = m_motionBlurEnabled ? 1 : 0;
            LOG_INFO(m_motionBlurEnabled ? "Motion Blur ON" : "Motion Blur OFF");
        }
        if (input.isKeyPressed(Key::G)) {
            m_dofEnabled      = !m_dofEnabled;
            m_dofData.enabled = m_dofEnabled ? 1 : 0;
            LOG_INFO(m_dofEnabled ? "DoF ON" : "DoF OFF");
        }
        if (input.isKeyPressed(Key::H)) {
            m_colorGradingEnabled              = !m_colorGradingEnabled;
            m_colorGradingData.lutEnabled      = m_colorGradingEnabled ? 1 : 0;
            LOG_INFO(m_colorGradingEnabled ? "Color Grading ON" : "Color Grading OFF");
        }
        if (input.isKeyPressed(Key::J)) {
            m_vignetteEnabled                  = !m_vignetteEnabled;
            m_vignetteData.vignetteEnabled     = m_vignetteEnabled ? 1 : 0;
            LOG_INFO(m_vignetteEnabled ? "Vignette ON" : "Vignette OFF");
        }
        if (input.isKeyPressed(Key::K)) {
            m_aberrationEnabled                = !m_aberrationEnabled;
            m_vignetteData.aberrationEnabled   = m_aberrationEnabled ? 1 : 0;
            LOG_INFO(m_aberrationEnabled ? "Chromatic Aberration ON" : "Chromatic Aberration OFF");
        }
        if (input.isKeyPressed(Key::N)) {
            m_bloomData.intensity = std::min(m_bloomData.intensity + 0.1f, 3.0f);
            LOG_INFO("Bloom intensity: " + std::to_string(m_bloomData.intensity));
        }
        if (input.isKeyPressed(Key::M)) {
            m_bloomData.intensity = std::max(m_bloomData.intensity - 0.1f, 0.0f);
            LOG_INFO("Bloom intensity: " + std::to_string(m_bloomData.intensity));
        }

        if (input.isKeyPressed(Key::F3)) {
            float& en = m_lightBuffer.lights[1].params.w;
            en = (en > 0.5f) ? 0.0f : 1.0f;
            LOG_INFO(en > 0.5f ? "Point light ON" : "Point light OFF");
        }
        if (input.isKeyPressed(Key::F4)) {
            float& en = m_lightBuffer.lights[2].params.w;
            en = (en > 0.5f) ? 0.0f : 1.0f;
            LOG_INFO(en > 0.5f ? "Spot light ON" : "Spot light OFF");
        }

        static float orbitAngle = 0.0f;
        orbitAngle += deltaTime;
        const float ptOrbitRadius = 2.5f;
        m_lightBuffer.lights[1].positionAndType = {
            ptOrbitRadius * cosf(orbitAngle),
            1.5f,
            ptOrbitRadius * sinf(orbitAngle),
            static_cast<float>(renderer::LightType::Point)
        };

        m_lightCB->update(m_lightBuffer);

        if (m_cubeEntity) {
            if (auto* t = m_cubeEntity->getComponent<Transform>())
                t->rotateEuler(deltaTime * 0.4f, deltaTime * 0.8f, deltaTime * 0.2f);
        }
    }
}