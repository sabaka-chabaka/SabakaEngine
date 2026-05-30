#include "core/MeshRenderer.h"
#include "core/Entity.h"
#include "core/Transform.h"
#include "core/SceneNode.h"
#include "core/BoundingBoxComponent.h"
#include "core/LodComponent.h"
#include "core/Logger.h"
#include <DirectXMath.h>

using namespace DirectX;

namespace engine::core {

    MeshRenderer::~MeshRenderer() = default;

    void MeshRenderer::setMesh(renderer::Mesh* mesh)                                          { m_mesh        = mesh; }
    void MeshRenderer::setMeshHandle(assets::AssetHandle<renderer::Mesh> handle)               { m_meshHandle  = std::move(handle); }
    void MeshRenderer::setMaterial(renderer::Material* material)                              { m_material    = material; }
    void MeshRenderer::setTransformCB(renderer::ConstantBuffer<renderer::TransformData>* cb)  { m_transformCB = cb; }
    void MeshRenderer::setLightCB(renderer::ConstantBuffer<renderer::LightBuffer>* cb)        { m_lightCB     = cb; }
    void MeshRenderer::setCamera(renderer::Camera* camera)                                    { m_camera      = camera; }
    void MeshRenderer::setFrustum(math::Frustum* frustum)                                     { m_frustum     = frustum; }

    renderer::Mesh*     MeshRenderer::getMesh()     const { return m_mesh; }
    renderer::Material* MeshRenderer::getMaterial() const { return m_material; }

    const assets::AssetHandle<renderer::Mesh>& MeshRenderer::getMeshHandle() const {
        return m_meshHandle;
    }

    void MeshRenderer::onRender() {
        if (!m_material || !m_transformCB || !m_camera || !owner) return;

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

        if (m_frustum) {
            if (auto* bb = owner->getComponent<BoundingBoxComponent>()) {
                if (!m_frustum->intersects(bb->getWorldAABB()))
                    return;
            }
        }

        renderer::Mesh* meshToDraw = m_mesh;

        if (!meshToDraw) {
            if (auto locked = m_meshHandle.lock())
                meshToDraw = locked.get();
        }

        if (auto* lod = owner->getComponent<LodComponent>()) {
            XMFLOAT3 camPos = m_camera->getPosition();
            XMVECTOR camVec = XMLoadFloat3(&camPos);

            XMFLOAT3 center;
            if (auto* bb = owner->getComponent<BoundingBoxComponent>()) {
                center = bb->getWorldAABB().center();
            } else {
                XMFLOAT3 worldPos;
                XMStoreFloat3(&worldPos, world.r[3]);
                center = worldPos;
            }

            XMVECTOR objVec  = XMLoadFloat3(&center);
            float    dist    = XMVectorGetX(XMVector3Length(XMVectorSubtract(camVec, objVec)));
            renderer::Mesh* lodMesh = lod->selectMesh(dist);
            if (lodMesh) meshToDraw = lodMesh;
        }

        if (!meshToDraw) return;

        renderer::TransformData td;
        td.model        = XMMatrixTranspose(world);
        td.view         = XMMatrixTranspose(m_camera->getViewMatrix());
        td.projection   = XMMatrixTranspose(m_camera->getProjectionMatrix());
        td.normalMatrix = XMMatrixTranspose(normalMatrix);

        m_transformCB->update(td);
        m_transformCB->bindVS(0);

        if (m_lightCB) m_lightCB->bindPS(2);

        m_material->bind();
        meshToDraw->draw();
    }
}