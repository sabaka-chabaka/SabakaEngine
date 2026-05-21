#include "SceneNode.h"
#include "Entity.h"
#include "Transform.h"
#include <algorithm>

namespace engine::core {
    SceneNode::~SceneNode() {
        detachFromParent();
        for (SceneNode* child: m_children) {
            child->m_parent = nullptr;
        }
    }

    void SceneNode::setParent(SceneNode* parent) {
        if (m_parent == parent) return;
        detachFromParent();
        m_parent = parent;
        if (parent) parent->addChild(this);
    }

    void SceneNode::detachFromParent() {
        if (!m_parent) return;
        m_parent->removeChild(this);
        m_parent = nullptr;
    }

    void SceneNode::addChild(SceneNode* child) {
        if (!child) return;
        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it == m_children.end()) m_children.push_back(child);
    }

    void SceneNode::removeChild(SceneNode* child) {
        m_children.erase(
            std::remove(m_children.begin(), m_children.end(), child),
            m_children.end()
        );
    }

    SceneNode* SceneNode::getParent() const {
        return m_parent;
    }

    const std::vector<SceneNode*>& SceneNode::getChildren() const {
        return m_children;
    }

    bool SceneNode::isRoot() const {
        return m_parent == nullptr;
    }

    XMMATRIX SceneNode::getWorldMatrix() const {
        XMMATRIX local = XMMatrixIdentity();
        if (owner) {
            if (auto* t = owner->getComponent<Transform>()) {
                local = t->getWorldMatrix();
            }
        }
        if (m_parent) return local * m_parent->getWorldMatrix();
        return local;
    }

    XMMATRIX SceneNode::getNormalMatrix() const {
        XMMATRIX world = getWorldMatrix();
        return XMMatrixTranspose(XMMatrixInverse(nullptr, world));
    }
}
