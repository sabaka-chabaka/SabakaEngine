#pragma once
#include "GraphicsDevice.h"
#include "Shader.h"
#include "ConstantBuffer.h"
#include "TransformData.h"
#include <vector>

namespace engine::renderer {

    class DepthPrePass {
    public:
        DepthPrePass(ID3D11Device* device, ID3D11DeviceContext* context);
        ~DepthPrePass() = default;

        DepthPrePass(const DepthPrePass&)            = delete;
        DepthPrePass& operator=(const DepthPrePass&) = delete;

        void begin(GraphicsDevice* gfx);
        void end(GraphicsDevice* gfx);

        ConstantBuffer<TransformData>* getTransformCB();

    private:
        ID3D11DeviceContext*                         m_context;
        std::unique_ptr<Shader>                      m_shader;
        std::unique_ptr<ConstantBuffer<TransformData>> m_transformCB;
    };
}