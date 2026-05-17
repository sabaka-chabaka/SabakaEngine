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
        std::unique_ptr<renderer::ConstantBuffer<renderer::MaterialData>>   m_materialCB;
        std::unique_ptr<renderer::Texture2D>                                m_diffuseTexture;
        std::unique_ptr<renderer::Texture2D>                                m_specularTexture;
        std::unique_ptr<renderer::SamplerState>                             m_sampler;
    };
}