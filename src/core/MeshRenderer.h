#pragma once
#include "Component.h"
#include "renderer/Mesh.h"
#include "renderer/Material.h"
#include "renderer/ConstantBuffer.h"
#include "renderer/TransformData.h"
#include "renderer/Light.h"
#include "renderer/Camera.h"

namespace engine::core {

    class MeshRenderer : public Component {
    public:
        MeshRenderer() = default;
        ~MeshRenderer() override;

        void onRender() override;

        void setMesh(renderer::Mesh* mesh);
        void setMaterial(renderer::Material* material);
        void setTransformCB(renderer::ConstantBuffer<renderer::TransformData>* cb);
        void setLightCB(renderer::ConstantBuffer<renderer::LightBuffer>* cb);
        void setCamera(renderer::Camera* camera);

        renderer::Mesh*     getMesh()     const;
        renderer::Material* getMaterial() const;

    private:
        renderer::Mesh*     m_mesh     = nullptr;
        renderer::Material* m_material = nullptr;

        renderer::ConstantBuffer<renderer::TransformData>* m_transformCB = nullptr;
        renderer::ConstantBuffer<renderer::LightBuffer>*   m_lightCB     = nullptr;
        renderer::Camera*                                  m_camera      = nullptr;
    };
}