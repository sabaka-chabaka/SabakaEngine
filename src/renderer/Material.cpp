#include "Material.h"

namespace engine::renderer {

    Material::Material(ID3D11Device* device, ID3D11DeviceContext* context)
        : m_context(context)
        , m_constantBuffer(device, context)
    {}

    void Material::setShader(Shader* shader)           { m_shader   = shader;   }
    void Material::setDiffuseTexture(Texture2D* t)     { m_diffuse  = t;        }
    void Material::setSpecularTexture(Texture2D* t)    { m_specular = t;        }
    void Material::setNormalMap(Texture2D* t)          { m_normalMap = t;       }
    void Material::setSampler(SamplerState* sampler)   { m_sampler  = sampler;  }
    void Material::setData(const MaterialData& data)   { m_data     = data;     }

    Shader*       Material::getShader()          const { return m_shader;    }
    Texture2D*    Material::getDiffuseTexture()  const { return m_diffuse;   }
    Texture2D*    Material::getSpecularTexture() const { return m_specular;  }
    Texture2D*    Material::getNormalMap()       const { return m_normalMap; }
    SamplerState* Material::getSampler()         const { return m_sampler;   }
    MaterialData& Material::getData()                  { return m_data;      }

    void Material::bind() const {
        if (m_shader)   m_shader->bind(m_context);
        if (m_diffuse)  m_diffuse->bindPS(0);
        if (m_specular) m_specular->bindPS(1);
        if (m_normalMap) m_normalMap->bindPS(2);
        if (m_sampler)  m_sampler->bindPS(0);

        m_constantBuffer.update(m_data);
        m_constantBuffer.bindPS(1);
    }
}