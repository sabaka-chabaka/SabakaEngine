#include "core/LodComponent.h"
#include <algorithm>

namespace engine::core {

    LodComponent::~LodComponent() = default;

    void LodComponent::addLevel(renderer::Mesh* mesh, float maxDistance) {
        m_levels.push_back({ mesh, maxDistance });
        std::sort(m_levels.begin(), m_levels.end(),
            [](const LodLevel& a, const LodLevel& b) {
                return a.maxDistance < b.maxDistance;
            });
    }

    renderer::Mesh* LodComponent::selectMesh(float distanceFromCamera) const {
        if (m_levels.empty()) return nullptr;

        for (auto& level : m_levels) {
            if (distanceFromCamera <= level.maxDistance)
                return level.mesh;
        }

        return m_levels.back().mesh;
    }

    int LodComponent::getLevelCount() const {
        return static_cast<int>(m_levels.size());
    }
}