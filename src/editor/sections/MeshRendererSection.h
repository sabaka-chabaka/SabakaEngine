#pragma once
#include "editor/sections/ComponentSection.h"

namespace engine::core { class MeshRenderer; }

namespace engine::editor {

    class MeshRendererSection : public ComponentSection {
        Q_OBJECT
    public:
        explicit MeshRendererSection(core::MeshRenderer* mr, QWidget* parent = nullptr);
    };

}
