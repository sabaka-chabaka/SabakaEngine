#pragma once
#include <QScrollArea>

namespace engine::core  { class Entity; }
namespace engine::editor { class EditorApplication; }

namespace engine::editor {

    class InspectorWidget : public QScrollArea {
        Q_OBJECT
    public:
        explicit InspectorWidget(QWidget* parent = nullptr);

        void setEngine(EditorApplication* engine);
        void inspect(core::Entity* entity);
        void clear();

    private:
        EditorApplication* m_engine = nullptr;
    };

}