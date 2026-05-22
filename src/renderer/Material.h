#pragma once
#include "Shader.h"
#include "Texture2D.h"
#include "SamplerState.h"
#include "ConstantBuffer.h"
#include "DirectXMath.h"

namespace engine::renderer {
    struct alignas(16) MaterialData {
        float specularIntensity = 1.0f;
        float specularPower     = 32.0f;
        DirectX::XMFLOAT2 uvScale  = { 1.0f, 1.0f };
        DirectX::XMFLOAT2 uvOffset = { 0.0f, 0.0f };
        float useNormalMap         = 1.0f;
        float _pad                 = 0.0f;
    };

    static_assert(sizeof(MaterialData) % 16 == 0,
        "MaterialData must be 16-byte aligned");

    class Material {
    public:
        Material(ID3D11Device* device, ID3D11DeviceContext* context);
        ~Material() = default;

        Material(const Material&)            = delete;
        Material& operator=(const Material&) = delete;

        void setShader(Shader* shader);
        void setDiffuseTexture(Texture2D* texture);
        void setSpecularTexture(Texture2D* texture);
        void setNormalMap(Texture2D* texture);
        void setSampler(SamplerState* sampler);
        void setData(const MaterialData& data);

        Shader*       getShader()          const;
        Texture2D*    getDiffuseTexture()  const;
        Texture2D*    getSpecularTexture() const;
        Texture2D*    getNormalMap()       const;
        SamplerState* getSampler()         const;
        MaterialData& getData();

        void bind() const;

    private:
        ID3D11DeviceContext* m_context = nullptr;

        Shader*       m_shader          = nullptr;
        Texture2D*    m_diffuse         = nullptr;
        Texture2D*    m_specular        = nullptr;
        Texture2D*    m_normalMap       = nullptr;
        SamplerState* m_sampler         = nullptr;

        MaterialData                          m_data;
        mutable ConstantBuffer<MaterialData>  m_constantBuffer;
    };
}
