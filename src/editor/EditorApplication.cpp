#define NOMINMAX
#include "editor/EditorApplication.h"
#include "core/Logger.h"
#include "core/Entity.h"
#include "core/Transform.h"
#include "core/MeshRenderer.h"
#include "core/BoundingBoxComponent.h"
#include "math/AABB.h"
#include "renderer/GraphicsDevice.h"

#include "platform/Input.h"
#include <stdexcept>

namespace engine::editor {

    EditorApplication::EditorApplication(HWND viewportHwnd, int width, int height)
        : Application(EditorTag{})
    {
        m_dofEnabled = false;

        core::Logger::get().openLogFile("editor.log");
        core::Logger::get().setMinLevel(core::LogLevel::Debug);

        renderer::GraphicsDeviceDesc desc;
        desc.hwnd   = viewportHwnd;
        desc.width  = static_cast<uint32_t>(width);
        desc.height = static_cast<uint32_t>(height);
        desc.vsync  = false;

        m_graphics = std::make_unique<renderer::GraphicsDevice>(desc);
        LOG_INFO("[EditorApplication] GraphicsDevice on Qt HWND");

        platform::InputSystem::get().initialize(nullptr);

        initCoreSystems();
        initRenderPipeline(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

        onInit();
    }

    void EditorApplication::onInit() {
        setupPrimitiveResources();
        LOG_INFO("[EditorApplication] ready");
    }

    void EditorApplication::setupPrimitiveResources() {
        auto* dev = m_graphics->getDevice();
        auto* ctx = m_graphics->getDeviceContext();

        m_cubeMesh = std::make_unique<renderer::Mesh>(
            renderer::Mesh::createCube(dev, ctx));

        std::vector<renderer::InputElementDesc> layout = {
            {"POSITION", DXGI_FORMAT_R32G32B32_FLOAT,    0},
            {"COLOR",    DXGI_FORMAT_R32G32B32_FLOAT,    12},
            {"TEXCOORD", DXGI_FORMAT_R32G32_FLOAT,       24},
            {"NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT,    32},
            {"TANGENT",  DXGI_FORMAT_R32G32B32A32_FLOAT, 44},
        };

        m_flatShader = std::make_unique<renderer::Shader>(
            dev,
            L"shaders/Basic.vs.hlsl",
            L"shaders/FlatColor.ps.hlsl",
            layout);

        renderer::SamplerDesc sampDesc;
        sampDesc.filter = renderer::FilterMode::Trilinear;
        sampDesc.wrapU  = renderer::WrapMode::Repeat;
        sampDesc.wrapV  = renderer::WrapMode::Repeat;
        m_defaultSampler = std::make_unique<renderer::SamplerState>(dev, ctx, sampDesc);

        m_defaultMaterial = std::make_unique<renderer::Material>(dev, ctx);
        m_defaultMaterial->setShader(m_flatShader.get());
        m_defaultMaterial->setSampler(m_defaultSampler.get());

        renderer::MaterialData matData;
        matData.useNormalMap = 0.f;
        m_defaultMaterial->setData(matData);

        LOG_INFO("[EditorApplication] primitive resources ready");
    }

    core::Entity* EditorApplication::createCube(const std::string& name) {
        core::Entity* e = m_scene->createEntity(name);

        auto* t = e->addComponent<core::Transform>();
        t->setPosition(0.f, 0.f, 0.f);
        t->setScale(1.f, 1.f, 1.f);

        auto* mr = e->addComponent<core::MeshRenderer>();
        mr->setMesh(m_cubeMesh.get());
        mr->setMaterial(m_defaultMaterial.get());
        mr->setTransformCB(m_transformCB.get());
        mr->setLightCB(m_lightCB.get());
        mr->setCamera(m_camera.get());
        mr->setFrustum(&m_frustum);

        auto* bb = e->addComponent<core::BoundingBoxComponent>();
        math::AABB aabb;
        aabb.min = { -0.5f, -0.5f, -0.5f };
        aabb.max = {  0.5f,  0.5f,  0.5f };
        bb->setLocalAABB(aabb);

        m_selectedEntity = e;
        LOG_INFO("[EditorApplication] created cube: " + name);
        return e;
    }

    core::Entity* EditorApplication::createEmpty(const std::string& name) {
        core::Entity* e = m_scene->createEntity(name);
        e->addComponent<core::Transform>();
        m_selectedEntity = e;
        LOG_INFO("[EditorApplication] created entity: " + name);
        return e;
    }

    void EditorApplication::destroyEntity(core::Entity* entity) {
        if (!entity) return;
        if (m_selectedEntity == entity) m_selectedEntity = nullptr;
        m_scene->destroyEntity(entity);
    }

    void EditorApplication::tick(float deltaTime) {
        m_graphics->beginFrame(0.08f, 0.08f, 0.12f);
        renderFrame(deltaTime);
        m_graphics->endFrame();
    }

}