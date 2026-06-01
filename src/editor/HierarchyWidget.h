#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QTimer>
#include <unordered_map>
#include <cstdint>

namespace engine::core { class Entity; }
namespace engine::editor { class EditorApplication; }

namespace engine::editor {

    class HierarchyWidget : public QWidget {
        Q_OBJECT
    public:
        explicit HierarchyWidget(QWidget* parent = nullptr);

        void setEngine(EditorApplication* engine);

        signals:
            void entitySelected(core::Entity* entity);
        void entityCreated(core::Entity* entity);
        void entityDeleted(core::Entity* entity);

    private slots:
        void onRefresh();
        void onItemClicked(QTreeWidgetItem* item, int column);
        void onItemDoubleClicked(QTreeWidgetItem* item, int column);
        void onContextMenu(const QPoint& pos);
        void onCreateCube();
        void onCreateEmpty();
        void onDeleteSelected();
        void onRenameSelected();

    private:
        void rebuild();
        QTreeWidgetItem* itemForEntity(core::Entity* entity);

        EditorApplication*                          m_engine  = nullptr;
        QTreeWidget*                                m_tree    = nullptr;
        QTimer                                      m_refreshTimer;
        std::unordered_map<uint64_t, QTreeWidgetItem*> m_itemMap;
    };

}