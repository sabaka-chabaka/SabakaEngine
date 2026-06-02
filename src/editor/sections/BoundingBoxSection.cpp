#include "editor/sections/BoundingBoxSection.h"
#include "core/BoundingBoxComponent.h"
#include "math/AABB.h"
#include <QLabel>

namespace engine::editor {

    BoundingBoxSection::BoundingBoxSection(core::BoundingBoxComponent* bb, QWidget* parent)
        : ComponentSection("Bounding Box", parent)
    {
        auto* form = formLayout();

        auto makeVec3Label = [](const DirectX::XMFLOAT3& v) {
            return new QLabel(QString("%1, %2, %3")
                .arg(v.x, 0, 'f', 2)
                .arg(v.y, 0, 'f', 2)
                .arg(v.z, 0, 'f', 2));
        };

        const auto& aabb = bb->getLocalAABB();
        form->addRow("Min (local)", makeVec3Label(aabb.min));
        form->addRow("Max (local)", makeVec3Label(aabb.max));

        const auto world = bb->getWorldAABB();
        form->addRow("Min (world)", makeVec3Label(world.min));
        form->addRow("Max (world)", makeVec3Label(world.max));
    }

}
