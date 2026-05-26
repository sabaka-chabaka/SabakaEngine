#pragma once
#include "RenderTarget.h"
#include "GraphicsDevice.h"
#include "Shader.h"
#include "SamplerState.h"
#include <memory>
#include <string>

namespace engine::renderer {

    class PostProcessPass {
    public:
        PostProcessPass(ID3D11Device* device, ID3D11DeviceContext* context, const std::wstring& psPath);
        ~PostProcessPass() = default;

        PostProcessPass(const PostProcessPass&)            = delete;
        PostProcessPass& operator=(const PostProcessPass&) = delete;

        void render(RenderTarget* input, RenderTarget* output, GraphicsDevice* gfx);
        void renderToBackBuffer(RenderTarget* input, GraphicsDevice* gfx);

        Shader* getShader();

    private:
        void drawFullscreenTriangle();

        ID3D11DeviceContext*          m_context;
        std::unique_ptr<Shader>       m_shader;
        std::unique_ptr<SamplerState> m_sampler;
    };
}