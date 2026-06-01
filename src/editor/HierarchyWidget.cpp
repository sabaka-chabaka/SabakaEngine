#include "editor/HierarchyWidget.h"
#include "editor/EditorApplication.h"
#include "core/Entity.h"
#include "core/Scene.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QLabel>
#include <QHeaderView>

namespace engine::editor {

    HierarchyWidget::HierarchyWidget(QWidget* parent)
        : QWidget(parent)
    {
        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(0);

        auto* header = new QWidget();
        header->setFixedHeight(28);
        auto* hl = new QHBoxLayout(header);
        hl->setContentsMargins(6, 2, 4, 2);

        auto* label = new QLabel("Scene");
        label->setStyleSheet("font-weight: bold;");
        hl->addWidget(label);
        hl->addStretch();

        auto* btnAdd = new QToolButton();
        btnAdd->setText("+");
        btnAdd->setFixedSize(22, 22);
        btnAdd->setToolTip("Create entity");
        connect(btnAdd, &QToolButton::clicked, this, [this, btnAdd]() {
            QMenu menu(this);
            menu.addAction(QIcon(), "Cube",        this, &HierarchyWidget::onCreateCube);
            menu.addAction(QIcon(), "Empty Entity", this, &HierarchyWidget::onCreateEmpty);
            menu.exec(btnAdd->mapToGlobal(QPoint(0, btnAdd->height())));
        });
        hl->addWidget(btnAdd);

        root->addWidget(header);

        m_tree = new QTreeWidget();
        m_tree->setHeaderHidden(true);
        m_tree->setAlternatingRowColors(true);
        m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
        m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
        m_tree->setDragDropMode(QAbstractItemView::InternalMove);

        connect(m_tree, &QTreeWidget::itemClicked,
                this, &HierarchyWidget::onItemClicked);
        connect(m_tree, &QTreeWidget::itemDoubleClicked,
                this, &HierarchyWidget::onItemDoubleClicked);
        connect(m_tree, &QTreeWidget::customContextMenuRequested,
                this, &HierarchyWidget::onContextMenu);

        root->addWidget(m_tree);

        m_refreshTimer.setInterval(200);
        connect(&m_refreshTimer, &QTimer::timeout, this, &HierarchyWidget::onRefresh);
    }

    void HierarchyWidget::setEngine(EditorApplication* engine) {
        m_engine = engine;
        if (m_engine)
            m_refreshTimer.start();
        else
            m_refreshTimer.stop();
        rebuild();
    }

    void HierarchyWidget::onRefresh() {
        if (!m_engine) return;
        rebuild();
    }

    void HierarchyWidget::rebuild() {
        if (!m_engine || !m_engine->getScene()) {
            m_tree->clear();
            m_itemMap.clear();
            return;
        }

        const auto& entities = m_engine->getScene()->getEntities();
        core::Entity* selected = m_engine->getSelectedEntity();

        m_tree->blockSignals(true);
        m_tree->clear();
        m_itemMap.clear();

        for (const auto& e : entities) {
            auto* item = new QTreeWidgetItem(m_tree);
            item->setText(0, QString::fromStdString(e->getName()));
            item->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(e.get())));

            if (e.get() == selected) {
                m_tree->setCurrentItem(item);
                item->setSelected(true);
            }

            m_itemMap[e->getId()] = item;
        }

        m_tree->blockSignals(false);
    }

    QTreeWidgetItem* HierarchyWidget::itemForEntity(core::Entity* entity) {
        if (!entity) return nullptr;
        auto it = m_itemMap.find(entity->getId());
        return it != m_itemMap.end() ? it->second : nullptr;
    }

    void HierarchyWidget::onItemClicked(QTreeWidgetItem* item, int) {
        if (!item || !m_engine) return;
        auto ptr = item->data(0, Qt::UserRole).value<quintptr>();
        auto* entity = reinterpret_cast<core::Entity*>(ptr);
        m_engine->selectEntity(entity);
        emit entitySelected(entity);
    }

    void HierarchyWidget::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
        onRenameSelected();
    }

    void HierarchyWidget::onContextMenu(const QPoint& pos) {
        QMenu menu(this);
        menu.addAction("Create Cube",        this, &HierarchyWidget::onCreateCube);
        menu.addAction("Create Empty Entity", this, &HierarchyWidget::onCreateEmpty);

        QTreeWidgetItem* item = m_tree->itemAt(pos);
        if (item) {
            menu.addSeparator();
            menu.addAction("Rename", this, &HierarchyWidget::onRenameSelected);
            menu.addAction("Delete", this, &HierarchyWidget::onDeleteSelected);
        }

        menu.exec(m_tree->viewport()->mapToGlobal(pos));
    }

    void HierarchyWidget::onCreateCube() {
        if (!m_engine) return;
        core::Entity* e = m_engine->createCube();
        rebuild();
        if (auto* item = itemForEntity(e))
            m_tree->setCurrentItem(item);
        emit entityCreated(e);
    }

    void HierarchyWidget::onCreateEmpty() {
        if (!m_engine) return;
        core::Entity* e = m_engine->createEmpty();
        rebuild();
        if (auto* item = itemForEntity(e))
            m_tree->setCurrentItem(item);
        emit entityCreated(e);
    }

    void HierarchyWidget::onDeleteSelected() {
        if (!m_engine) return;
        QTreeWidgetItem* item = m_tree->currentItem();
        if (!item) return;
        auto ptr = item->data(0, Qt::UserRole).value<quintptr>();
        auto* entity = reinterpret_cast<core::Entity*>(ptr);
        emit entityDeleted(entity);
        m_engine->destroyEntity(entity);
        rebuild();
    }

    void HierarchyWidget::onRenameSelected() {
        if (!m_engine) return;
        QTreeWidgetItem* item = m_tree->currentItem();
        if (!item) return;
        auto ptr = item->data(0, Qt::UserRole).value<quintptr>();
        auto* entity = reinterpret_cast<core::Entity*>(ptr);

        bool ok = false;
        QString newName = QInputDialog::getText(
            this, "Rename Entity",
            "Name:", QLineEdit::Normal,
            QString::fromStdString(entity->getName()), &ok);

        if (ok && !newName.isEmpty()) {
            entity->setName(newName.toStdString());
            item->setText(0, newName);
        }
    }

}