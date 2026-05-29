#pragma once
#include "core/Component.h"
#include "renderer/Mesh.h"
#include "renderer/Material.h"
#include "renderer/ConstantBuffer.h"
#include "renderer/TransformData.h"
#include "renderer/Light.h"
#include "renderer/Camera.h"
#include "math/Frustum.h"
#include "assets/AssetHandle.h"

namespace engine::core {

    class MeshRenderer : public Component {
    public:
        MeshRenderer() = default;
        ~MeshRenderer() override;

        void onRender() override;

        void setMesh(renderer::Mesh* mesh);
        void setMeshHandle(assets::AssetHandle<renderer::Mesh> handle);
        void setMaterial(renderer::Material* material);
        void setTransformCB(renderer::ConstantBuffer<renderer::TransformData>* cb);
        void setLightCB(renderer::ConstantBuffer<renderer::LightBuffer>* cb);
        void setCamera(renderer::Camera* camera);
        void setFrustum(math::Frustum* frustum);

        renderer::Mesh*                              getMesh()       const;
        renderer::Material*                          getMaterial()   const;
        const assets::AssetHandle<renderer::Mesh>&    getMeshHandle() const;

    private:
        renderer::Mesh*                           m_mesh       = nullptr;
        assets::AssetHandle<renderer::Mesh>        m_meshHandle;
        renderer::Material*                       m_material   = nullptr;

        renderer::ConstantBuffer<renderer::TransformData>* m_transformCB = nullptr;
        renderer::ConstantBuffer<renderer::LightBuffer>*   m_lightCB     = nullptr;
        renderer::Camera*                                  m_camera      = nullptr;
        math::Frustum*                                     m_frustum     = nullptr;
    };
}