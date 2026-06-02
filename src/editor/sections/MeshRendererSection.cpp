#include "editor/sections/MeshRendererSection.h"
#include "core/MeshRenderer.h"
#include "renderer/Mesh.h"
#include "renderer/Material.h"
#include <QLabel>
#include <QLineEdit>

namespace engine::editor {

    MeshRendererSection::MeshRendererSection(core::MeshRenderer* mr, QWidget* parent)
        : ComponentSection("Mesh Renderer", parent)
    {
        auto* form = formLayout();

        auto makeReadOnly = [&](const QString& text) {
            auto* le = new QLineEdit(text, this);
            le->setReadOnly(true);
            le->setStyleSheet("QLineEdit { background: palette(button); }");
            return le;
        };

        const QString meshName = mr->getMesh() ? "mesh" : "(none)";
        form->addRow("Mesh", makeReadOnly(meshName));

        const QString matName = mr->getMaterial() ? "default" : "(none)";
        form->addRow("Material", makeReadOnly(matName));
    }

}
