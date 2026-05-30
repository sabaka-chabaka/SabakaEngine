#pragma once
#include <QMainWindow>
#include <QDockWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QScrollArea>
#include <memory>

namespace engine::editor {

    class ViewportWindow;

    class MainWindow : public QMainWindow {
        Q_OBJECT
    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override = default;

    private:
        void setupMenuBar();
        void setupToolBar();
        void setupDocks();
        void setupCentralViewport();

        QDockWidget* makeDock(const QString& title, QWidget* contents,
                              Qt::DockWidgetArea area);

        ViewportWindow*  m_viewportWindow = nullptr;
        QWidget*         m_viewportContainer = nullptr;

        QTreeWidget*     m_hierarchyTree = nullptr;
        QScrollArea*     m_inspectorArea = nullptr;
        QListWidget*     m_assetList     = nullptr;
    };

}