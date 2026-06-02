#include "editor/InspectorWidget.h"
#include "editor/sections/TransformSection.h"
#include "editor/sections/MeshRendererSection.h"
#include "editor/sections/BoundingBoxSection.h"
#include "editor/EditorApplication.h"
#include "core/Entity.h"
#include "core/Transform.h"
#include "core/MeshRenderer.h"
#include "core/BoundingBoxComponent.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace engine::editor {

    InspectorWidget::InspectorWidget(QWidget* parent)
        : QScrollArea(parent)
    {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setWidgetResizable(true);
        clear();
    }

    void InspectorWidget::setEngine(EditorApplication* engine) {
        m_engine = engine;
    }

    void InspectorWidget::inspect(core::Entity* entity) {
        if (!entity) {
            clear();
            return;
        }

        auto* root   = new QWidget();
        auto* layout = new QVBoxLayout(root);
        layout->setContentsMargins(6, 6, 6, 6);
        layout->setSpacing(4);
        layout->setAlignment(Qt::AlignTop);

        auto* nameLabel = new QLabel(QString("<b>%1</b>").arg(
            QString::fromStdString(entity->getName())));
        nameLabel->setContentsMargins(2, 0, 0, 4);
        layout->addWidget(nameLabel);

        if (auto* t = entity->getComponent<core::Transform>()) {
            layout->addWidget(new TransformSection(t, root));
        }

        if (auto* mr = entity->getComponent<core::MeshRenderer>()) {
            layout->addWidget(new MeshRendererSection(mr, root));
        }

        if (auto* bb = entity->getComponent<core::BoundingBoxComponent>()) {
            layout->addWidget(new BoundingBoxSection(bb, root));
        }

        layout->addStretch();
        setWidget(root);
    }

    void InspectorWidget::clear() {
        auto* placeholder = new QLabel("Select an entity to inspect");
        placeholder->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        placeholder->setContentsMargins(8, 16, 8, 8);
        setWidget(placeholder);
    }

}