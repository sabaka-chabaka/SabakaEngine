#pragma once
#include "core/Application.h"
#include "renderer/Mesh.h"
#include "renderer/Shader.h"
#include "renderer/Texture2D.h"
#include "renderer/Material.h"
#include "renderer/SamplerState.h"
#include <memory>

namespace engine::runtime {

    class EngineRuntime : public core::Application {
    public:
        EngineRuntime();
        ~EngineRuntime() = default;

    protected:
        void onInit()                  override;
        void onUpdate(float deltaTime) override;

    private:
        std::unique_ptr<renderer::Mesh>      m_mesh;
        std::unique_ptr<renderer::Shader>    m_shader;
        std::unique_ptr<renderer::Texture2D> m_diffuseTexture;
        std::unique_ptr<renderer::Texture2D> m_specularTexture;
        std::unique_ptr<renderer::Texture2D> m_normalMap;
        std::unique_ptr<renderer::SamplerState> m_sampler;
        std::unique_ptr<renderer::Material>  m_cubeMaterial;

        core::Entity* m_cubeEntity = nullptr;
    };
}