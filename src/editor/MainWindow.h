#pragma once
#include <QMainWindow>
#include <QAction>
#include <QDockWidget>
#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QToolBar>
#include <QTreeWidget>

namespace engine::editor {

    class ViewportWindow;

    class MainWindow : public QMainWindow {
        Q_OBJECT
    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override = default;

    private slots:
        void onNewScene();
        void onOpenScene();
        void onSaveScene();
        void onPlay();
        void onPause();
        void onStop();
        void onGizmoMove();
        void onGizmoRotate();
        void onGizmoScale();
        void onAbout();

    private:
        void setupMenuBar();
        void setupToolBar();
        void setupDocks();
        void setupCentralViewport();

        QDockWidget* makeDock(const QString& title, QWidget* contents,
                              Qt::DockWidgetArea area, int minW = 200, int minH = 120);

        ViewportWindow* m_viewportWindow    = nullptr;
        QWidget*        m_viewportContainer = nullptr;

        QTreeWidget*  m_hierarchyTree = nullptr;
        QScrollArea*  m_inspectorArea = nullptr;
        QListWidget*  m_assetList     = nullptr;

        QAction* m_actPlay   = nullptr;
        QAction* m_actPause  = nullptr;
        QAction* m_actStop   = nullptr;
        QAction* m_actMove   = nullptr;
        QAction* m_actRotate = nullptr;
        QAction* m_actScale  = nullptr;

        bool m_playing = false;
    };

}