#pragma once
#include "core/Component.h"
#include "renderer/Mesh.h"
#include <vector>

namespace engine::core {

    class LodComponent : public Component {
    public:
        LodComponent() = default;
        ~LodComponent() override;

        void addLevel(renderer::Mesh* mesh, float maxDistance);

        renderer::Mesh* selectMesh(float distanceFromCamera) const;
        int             getLevelCount() const;

    private:
        struct LodLevel {
            renderer::Mesh* mesh;
            float           maxDistance;
        };

        std::vector<LodLevel> m_levels;
    };
}