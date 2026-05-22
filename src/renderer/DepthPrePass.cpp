#include "DepthPrePass.h"

#include <memory>

namespace engine::renderer {

    DepthPrePass::DepthPrePass(ID3D11Device* device, ID3D11DeviceContext* context)
        : m_context(context)
    {
        m_transformCB = std::make_unique<ConstantBuffer<TransformData>>(device, context);
    }

    void DepthPrePass::begin(GraphicsDevice* gfx) {
        gfx->setColorWriteEnabled(false);
        gfx->setDepthWriteEnabled(true);
        gfx->setDepthFunc(DepthFunc::Less);
    }

    void DepthPrePass::end(GraphicsDevice* gfx) {
        gfx->setColorWriteEnabled(true);
        gfx->setDepthFunc(DepthFunc::LessEqual);
    }

    ConstantBuffer<TransformData>* DepthPrePass::getTransformCB() {
        return m_transformCB.get();
    }
}
