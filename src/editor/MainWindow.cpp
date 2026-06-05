#include "editor/MainWindow.h"
#include "editor/PlayModeManager.h"
#include "editor/SceneSerializer.h"
#include "editor/commands/CommandHistory.h"
#include "editor/commands/EntityCommands.h"
#include "editor/GizmoMode.h"
#include "editor/ViewportWindow.h"
#include "editor/HierarchyWidget.h"
#include "editor/InspectorWidget.h"
#include "editor/AssetBrowserWidget.h"
#include "editor/ViewportDropHandler.h"
#include "editor/EditorApplication.h"
#include "core/Entity.h"
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSizePolicy>
#include <QStatusBar>

namespace engine::editor {

    MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
    {
        setWindowTitle("SabakaEngine Editor");
        setMinimumSize(1024, 600);
        resize(1600, 900);

        m_playMode = std::make_unique<PlayModeManager>();

        setupToolBar();
        setupMenuBar();
        setupDocks();
        setupCentralViewport();

        statusBar()->showMessage("Ready");
    }

    void MainWindow::setupMenuBar() {
        QMenu* file = menuBar()->addMenu("File");
        file->addAction("New Scene",  this, &MainWindow::onNewScene,  QKeySequence::New);
        file->addAction("Open Scene", this, &MainWindow::onOpenScene, QKeySequence::Open);
        file->addAction("Save Scene", this, &MainWindow::onSaveScene, QKeySequence::Save);
        file->addSeparator();
        file->addAction("Exit", QApplication::instance(), &QApplication::quit, QKeySequence::Quit);

        QMenu* edit = menuBar()->addMenu("Edit");
        edit->addAction("Undo", QKeySequence::Undo, [this] {
            CommandHistory::get().undo();
            statusBar()->showMessage(QString("Undo: %1").arg(
                QString::fromStdString(CommandHistory::get().redoDescription())));
        });
        edit->addAction("Redo", QKeySequence::Redo, [this] {
            CommandHistory::get().redo();
            statusBar()->showMessage(QString("Redo: %1").arg(
                QString::fromStdString(CommandHistory::get().undoDescription())));
        });
        edit->addSeparator();
        QMenu* gizmoMenu = edit->addMenu("Gizmo Mode");
        gizmoMenu->addAction(m_actMove);
        gizmoMenu->addAction(m_actRotate);
        gizmoMenu->addAction(m_actScale);

        QMenu* view = menuBar()->addMenu("View");
        view->addAction("Hierarchy",     [this] {
            if (m_hierarchy) m_hierarchy->parentWidget()->parentWidget()->show();
        });
        view->addAction("Inspector",     [this] {
            if (m_inspector) m_inspector->parentWidget()->parentWidget()->show();
        });
        view->addAction("Asset Browser", [this] {
            if (m_assetBrowser) m_assetBrowser->parentWidget()->parentWidget()->show();
        });

        menuBar()->addMenu("Help")->addAction("About", this, &MainWindow::onAbout);
    }

    void MainWindow::setupToolBar() {
        QToolBar* tb = addToolBar("Main");
        tb->setObjectName("mainToolBar");
        tb->setMovable(false);

        m_actPlay  = tb->addAction("▶  Play",  this, &MainWindow::onPlay);
        m_actPause = tb->addAction("⏸  Pause", this, &MainWindow::onPause);
        m_actStop  = tb->addAction("⏹  Stop",  this, &MainWindow::onStop);
        m_actPause->setEnabled(false);
        m_actStop->setEnabled(false);

        tb->addSeparator();

        m_actMove   = new QAction("Move", this);
        m_actRotate = new QAction("Rotate", this);
        m_actScale  = new QAction("Scale", this);

        // NOTE: W/E/R shortcuts are intentionally NOT set here — they would steal
        // keyboard input from the viewport (camera WASD movement). Gizmo mode can
        // be switched via the toolbar buttons or the Edit > Gizmo Mode menu.
        // m_actMove->setShortcut(QKeySequence("W"));
        // m_actRotate->setShortcut(QKeySequence("E"));
        // m_actScale->setShortcut(QKeySequence("R"));

        m_actMove->setCheckable(true);
        m_actRotate->setCheckable(true);
        m_actScale->setCheckable(true);
        m_actMove->setChecked(true);

        connect(m_actMove,   &QAction::triggered, this, &MainWindow::onGizmoMove);
        connect(m_actRotate, &QAction::triggered, this, &MainWindow::onGizmoRotate);
        connect(m_actScale,  &QAction::triggered, this, &MainWindow::onGizmoScale);

        tb->addAction(m_actMove);
        tb->addAction(m_actRotate);
        tb->addAction(m_actScale);
    }

    void MainWindow::setupDocks() {
        m_hierarchy = new HierarchyWidget();
        connect(m_hierarchy, &HierarchyWidget::entitySelected,
                this, &MainWindow::onEntitySelected);
        makeDock("Hierarchy", m_hierarchy, Qt::LeftDockWidgetArea, 220, 300);

        m_inspector = new InspectorWidget();
        makeDock("Inspector", m_inspector, Qt::RightDockWidgetArea, 260, 300);

        m_assetBrowser = new AssetBrowserWidget();
        m_assetBrowser->setRootPath(QDir::currentPath());
        makeDock("Asset Browser", m_assetBrowser, Qt::BottomDockWidgetArea, 0, 180);
    }

    void MainWindow::setupCentralViewport() {
        m_viewportWindow = new ViewportWindow();
        connect(m_viewportWindow, &ViewportWindow::engineReady,
                this, &MainWindow::onEngineReady);
        connect(m_viewportWindow, &ViewportWindow::rightMouseButtonChanged,
                this, &MainWindow::onViewportMouseButtonChanged);

        m_viewportContainer = QWidget::createWindowContainer(m_viewportWindow, this);
        m_viewportContainer->setMinimumSize(320, 240);
        m_viewportContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_viewportContainer->setFocusPolicy(Qt::StrongFocus);
        m_viewportContainer->setAcceptDrops(true);

        m_dropHandler = new ViewportDropHandler(this);
        m_viewportContainer->installEventFilter(m_dropHandler);
        connect(m_dropHandler, &ViewportDropHandler::entityDropped,
                this, &MainWindow::onEntityDropped);

        setCentralWidget(m_viewportContainer);
    }

    QDockWidget* MainWindow::makeDock(const QString& title, QWidget* contents,
                                      Qt::DockWidgetArea area, int minW, int minH) {
        auto* dock = new QDockWidget(title, this);
        dock->setObjectName(title);
        dock->setWidget(contents);
        dock->setFeatures(QDockWidget::DockWidgetMovable |
                          QDockWidget::DockWidgetFloatable |
                          QDockWidget::DockWidgetClosable);
        if (minW > 0) dock->setMinimumWidth(minW);
        if (minH > 0) dock->setMinimumHeight(minH);
        addDockWidget(area, dock);
        return dock;
    }

    void MainWindow::onEngineReady(EditorApplication* engine) {
        m_hierarchy->setEngine(engine);
        m_inspector->setEngine(engine);
        m_dropHandler->setEngine(engine);
        m_playMode->setApp(engine);
        statusBar()->showMessage("Engine ready");
    }

    void MainWindow::onEntityDropped(EditorApplication* engine, const QString& assetPath) {
        m_hierarchy->setEngine(engine);
        core::Entity* sel = engine->getSelectedEntity();
        if (sel) {
            m_inspector->inspect(sel);
            statusBar()->showMessage(QString("Dropped: %1").arg(assetPath));
        }
    }

    void MainWindow::onEntitySelected(core::Entity* entity) {
        if (!entity) return;
        m_inspector->inspect(entity);
        statusBar()->showMessage(QString("Selected: %1")
            .arg(QString::fromStdString(entity->getName())));
    }

    void MainWindow::onNewScene() {
        auto* eng = m_viewportWindow ? m_viewportWindow->getEngine() : nullptr;
        if (eng) {
            eng->clearScene();
            m_hierarchy->setEngine(eng);
            m_inspector->clear();
            CommandHistory::get().clear();
        }
        statusBar()->showMessage("New scene");
    }

    void MainWindow::onOpenScene() {
        QString path = QFileDialog::getOpenFileName(
            this, "Open Scene", "", "Scene Files (*.scene);;All Files (*)");
        if (path.isEmpty()) return;
        auto* eng = m_viewportWindow ? m_viewportWindow->getEngine() : nullptr;
        if (!eng) return;
        eng->clearScene();
        SceneSerializer::loadFromFile(eng, path.toStdString());
        m_hierarchy->setEngine(eng);
        m_inspector->clear();
        CommandHistory::get().clear();
        statusBar()->showMessage("Opened: " + path);
    }

    void MainWindow::onSaveScene() {
        QString path = QFileDialog::getSaveFileName(
            this, "Save Scene", "", "Scene Files (*.scene);;All Files (*)");
        if (path.isEmpty()) return;
        auto* eng = m_viewportWindow ? m_viewportWindow->getEngine() : nullptr;
        if (!eng) return;
        SceneSerializer::saveToFile(eng->getScene(), path.toStdString());
        statusBar()->showMessage("Saved: " + path);
    }

    void MainWindow::onPlay() {
        m_playMode->play();
        m_actPlay->setEnabled(false);
        m_actPause->setEnabled(true);
        m_actStop->setEnabled(true);
        statusBar()->showMessage("Playing...");
    }

    void MainWindow::onPause() {
        m_playMode->pause();
        m_actPlay->setEnabled(true);
        m_actPause->setEnabled(false);
        statusBar()->showMessage("Paused");
    }

    void MainWindow::onStop() {
        m_playMode->stop();
        m_actPlay->setEnabled(true);
        m_actPause->setEnabled(false);
        m_actStop->setEnabled(false);
        if (auto* eng = m_viewportWindow->getEngine()) {
            m_hierarchy->setEngine(eng);
            m_inspector->clear();
        }
        statusBar()->showMessage("Stopped");
    }

    void MainWindow::onGizmoMove() {
        m_actMove->setChecked(true);
        m_actRotate->setChecked(false);
        m_actScale->setChecked(false);
        if (m_viewportWindow->getEngine())
            m_viewportWindow->getEngine()->setGizmoMode(GizmoMode::Translate);
        statusBar()->showMessage("Gizmo: Move");
    }

    void MainWindow::onGizmoRotate() {
        m_actMove->setChecked(false);
        m_actRotate->setChecked(true);
        m_actScale->setChecked(false);
        if (m_viewportWindow->getEngine())
            m_viewportWindow->getEngine()->setGizmoMode(GizmoMode::Rotate);
        statusBar()->showMessage("Gizmo: Rotate");
    }

    void MainWindow::onGizmoScale() {
        m_actMove->setChecked(false);
        m_actRotate->setChecked(false);
        m_actScale->setChecked(true);
        if (m_viewportWindow->getEngine())
            m_viewportWindow->getEngine()->setGizmoMode(GizmoMode::Scale);
        statusBar()->showMessage("Gizmo: Scale");
    }

    void MainWindow::onViewportMouseButtonChanged(bool pressed) {
        bool enabled = !pressed;
        m_actMove->setEnabled(enabled);
        m_actRotate->setEnabled(enabled);
        m_actScale->setEnabled(enabled);
    }

    void MainWindow::onAbout() {
        QMessageBox::about(this, "SabakaEngine",
            "SabakaEngine Editor\n\nDirectX 11 | PhysX | XAudio2\nC++20");
    }

}