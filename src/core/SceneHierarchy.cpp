#include "SceneHierarchy.h"

namespace engine::core {

    void SceneHierarchy::setParent(Entity* child, Entity* parent) {
        if (!child || !parent) return;
        SceneNode* childNode  = getOrCreateNode(child);
        SceneNode* parentNode = getOrCreateNode(parent);
        childNode->setParent(parentNode);
    }

    void SceneHierarchy::detach(Entity* child) {
        if (!child) return;
        SceneNode* node = getNode(child);
        if (node) node->detachFromParent();
    }

    SceneNode* SceneHierarchy::getNode(Entity* entity) const {
        if (!entity) return nullptr;
        return entity->getComponent<SceneNode>();
    }

    SceneNode* SceneHierarchy::getOrCreateNode(Entity* entity) const {
        SceneNode* node = entity->getComponent<SceneNode>();
        if (!node) node = entity->addComponent<SceneNode>();
        return node;
    }
}