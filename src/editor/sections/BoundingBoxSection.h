#pragma once
#include "editor/sections/ComponentSection.h"

namespace engine::core { class BoundingBoxComponent; }

namespace engine::editor {

    class BoundingBoxSection : public ComponentSection {
        Q_OBJECT
    public:
        explicit BoundingBoxSection(core::BoundingBoxComponent* bb, QWidget* parent = nullptr);
    };

}
