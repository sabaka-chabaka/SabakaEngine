#include "MeshRenderer.h"
#include "Entity.h"
#include "Transform.h"
#include "SceneNode.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace engine::core {

    MeshRenderer::~MeshRenderer() = default;

    void MeshRenderer::setMesh(renderer::Mesh* mesh)                                       { m_mesh        = mesh; }
    void MeshRenderer::setMaterial(renderer::Material* material)                           { m_material    = material; }
    void MeshRenderer::setTransformCB(renderer::ConstantBuffer<renderer::TransformData>* cb) { m_transformCB = cb; }
    void MeshRenderer::setLightCB(renderer::ConstantBuffer<renderer::LightBuffer>* cb)    { m_lightCB     = cb; }
    void MeshRenderer::setCamera(renderer::Camera* camera)                                 { m_camera      = camera; }

    renderer::Mesh*     MeshRenderer::getMesh()     const { return m_mesh; }
    renderer::Material* MeshRenderer::getMaterial() const { return m_material; }

    void MeshRenderer::onRender() {
        if (!m_mesh || !m_material || !m_transformCB || !m_camera || !owner) return;

        XMMATRIX world;
        XMMATRIX normalMatrix;

        if (auto* node = owner->getComponent<SceneNode>()) {
            world        = node->getWorldMatrix();
            normalMatrix = node->getNormalMatrix();
        } else if (auto* transform = owner->getComponent<Transform>()) {
            world        = transform->getWorldMatrix();
            normalMatrix = transform->getNormalMatrix();
        } else {
            world        = XMMatrixIdentity();
            normalMatrix = XMMatrixIdentity();
        }

        renderer::TransformData td;
        td.model        = XMMatrixTranspose(world);
        td.view         = XMMatrixTranspose(m_camera->getViewMatrix());
        td.projection   = XMMatrixTranspose(m_camera->getProjectionMatrix());
        td.normalMatrix = XMMatrixTranspose(normalMatrix);

        m_transformCB->update(td);
        m_transformCB->bindVS(0);

        if (m_lightCB) {
            m_lightCB->bindPS(2);
        }

        m_material->bind();
        m_mesh->draw();
    }
}