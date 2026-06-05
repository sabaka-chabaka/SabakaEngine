#pragma once
#include <QMainWindow>
#include <QAction>
#include <QDockWidget>
#include <QScrollArea>
#include <QToolBar>
#include <memory>

namespace engine::core  { class Entity; }
namespace engine::editor {
    class ViewportWindow;
    class HierarchyWidget;
    class InspectorWidget;
    class AssetBrowserWidget;
    class ViewportDropHandler;
    class PlayModeManager;
    class EditorApplication;
}

namespace engine::editor {

    class MainWindow : public QMainWindow {
        Q_OBJECT
    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override = default;

    private slots:
        void onEngineReady(EditorApplication* engine);
        void onEntitySelected(core::Entity* entity);
        void onEntityDropped(EditorApplication* engine, const QString& assetPath);
        void onNewScene();
        void onOpenScene();
        void onSaveScene();
        void onPlay();
        void onPause();
        void onStop();
        void onGizmoMove();
        void onGizmoRotate();
        void onGizmoScale();
        void onViewportMouseButtonChanged(bool pressed);
        void onAbout();

    private:
        void setupMenuBar();
        void setupToolBar();
        void setupDocks();
        void setupCentralViewport();

        QDockWidget* makeDock(const QString& title, QWidget* contents,
                              Qt::DockWidgetArea area, int minW = 200, int minH = 120);

        ViewportWindow*      m_viewportWindow    = nullptr;
        QWidget*             m_viewportContainer = nullptr;
        HierarchyWidget*     m_hierarchy         = nullptr;
        InspectorWidget*     m_inspector         = nullptr;
        AssetBrowserWidget*  m_assetBrowser      = nullptr;
        ViewportDropHandler* m_dropHandler       = nullptr;

        QAction* m_actPlay   = nullptr;
        QAction* m_actPause  = nullptr;
        QAction* m_actStop   = nullptr;
        QAction* m_actMove   = nullptr;
        QAction* m_actRotate = nullptr;
        QAction* m_actScale  = nullptr;

        std::unique_ptr<PlayModeManager> m_playMode;
    };

}
