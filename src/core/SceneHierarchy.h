#pragma once
#include "SceneNode.h"
#include "Entity.h"

namespace engine::core {

    class SceneHierarchy {
    public:
        SceneHierarchy()  = default;
        ~SceneHierarchy() = default;

        SceneHierarchy(const SceneHierarchy&)            = delete;
        SceneHierarchy& operator=(const SceneHierarchy&) = delete;

        void setParent(Entity* child, Entity* parent);
        void detach(Entity* child);

        SceneNode* getNode(Entity* entity) const;

    private:
        SceneNode* getOrCreateNode(Entity* entity) const;
    };
}