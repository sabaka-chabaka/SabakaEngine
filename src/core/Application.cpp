#include "core/Application.h"
#include "core/Logger.h"
#include "platform/Input.h"
#include <DirectXMath.h>
#include <chrono>

using namespace DirectX;
using namespace engine::platform;

namespace engine::core {
    Application::Application() {
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

        Logger::get().openLogFile("engine.log");
        Logger::get().setMinLevel(LogLevel::Info);

        LOG_INFO("Initializing SabakaEngine");

        platform::WindowDesc wDesc;
        wDesc.title = L"SabakaEngine v0.1";
        wDesc.width = 1280;
        wDesc.height = 720;

        m_window = std::make_unique<platform::Window>(wDesc);
        LOG_INFO("Window created (1280x720)");

        InputSystem::get().initialize(m_window->getNativeHandle());

        renderer::GraphicsDeviceDesc gDesc;
        gDesc.hwnd = m_window->getNativeHandle();
        gDesc.width = m_window->getWidth();
        gDesc.height = m_window->getHeight();
        gDesc.vsync = true;

        m_graphics = std::make_unique<renderer::GraphicsDevice>(gDesc);
        LOG_INFO("DirectX 11 device initialized");

        m_mesh = std::make_unique<renderer::Mesh>(
            renderer::Mesh::createCube(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext()
        )
);

        std::vector<renderer::InputElementDesc> layout = {
            {"POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0},
            {"COLOR", DXGI_FORMAT_R32G32B32_FLOAT, 12},
            {"TEXCOORD", DXGI_FORMAT_R32G32_FLOAT, 24},
        };

        m_shader = std::make_unique<renderer::Shader>(
            m_graphics->getDevice(),
            L"shaders/Basic.vs.hlsl",
            L"shaders/Basic.ps.hlsl",
            layout
        );

        renderer::CameraDesc camDesc;
        camDesc.position = {0.0f, 1.5f, -3.0f};
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

        m_transformCB = std::make_unique<renderer::ConstantBuffer<renderer::TransformData> >(
            m_graphics->getDevice(),
            m_graphics->getDeviceContext()
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

        m_graphics->setFillMode(renderer::FillMode::Solid);
        m_graphics->setCullMode(renderer::CullMode::None);

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
                    LOG_INFO("Wireframe mode");
                }
                if (input.isKeyPressed(Key::F2)) {
                    m_graphics->setFillMode(renderer::FillMode::Solid);
                    LOG_INFO("Solid mode");
                }

                onUpdate(deltaTime);

                XMMATRIX model = XMMatrixRotationRollPitchYaw(
                    totalTime * 0.4f,
                    totalTime * 0.8f,
                    totalTime * 0.2f
                );

                renderer::TransformData td;
                td.model = XMMatrixTranspose(model);
                td.view = XMMatrixTranspose(m_camera->getViewMatrix());
                td.projection = XMMatrixTranspose(m_camera->getProjectionMatrix());

                m_transformCB->update(td);
                m_transformCB->bindVS(0);

                m_graphics->beginFrame(0.08f, 0.08f, 0.12f);
                m_shader->bind(m_graphics->getDeviceContext());
                m_mesh->draw();
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
