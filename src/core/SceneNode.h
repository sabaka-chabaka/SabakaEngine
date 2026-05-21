#pragma once
#include "Component.h"
#include <DirectXMath.h>
#include <string>
#include <vector>

namespace engine::core {
    using namespace DirectX;

    class SceneNode : public Component {
    public:
        SceneNode()  = default;
        ~SceneNode() override;

        void setParent(SceneNode* parent);
        void detachFromParent();
        void addChild(SceneNode* child);
        void removeChild(SceneNode* child);

        SceneNode*              getParent()   const;
        const std::vector<SceneNode*>& getChildren() const;
        bool                    isRoot()      const;

        XMMATRIX getWorldMatrix()  const;
        XMMATRIX getNormalMatrix() const;

    private:
        SceneNode*              m_parent = nullptr;
        std::vector<SceneNode*> m_children;
    };
}
