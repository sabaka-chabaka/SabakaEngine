#include "editor/MainWindow.h"
#include "editor/ViewportWindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSizePolicy>
#include <QStatusBar>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace engine::editor {

    MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
    {
        setWindowTitle("SabakaEngine Editor");
        setMinimumSize(1024, 600);
        resize(1600, 900);

        setupMenuBar();
        setupToolBar();
        setupDocks();
        setupCentralViewport();

        statusBar()->showMessage("Ready");
    }

    void MainWindow::setupMenuBar() {
        QMenu* file = menuBar()->addMenu("File");
        file->addAction(QIcon(), "New Scene",  this, &MainWindow::onNewScene,  QKeySequence::New);
        file->addAction(QIcon(), "Open Scene", this, &MainWindow::onOpenScene, QKeySequence::Open);
        file->addAction(QIcon(), "Save Scene", this, &MainWindow::onSaveScene, QKeySequence::Save);
        file->addSeparator();
        file->addAction("Exit", QApplication::instance(), &QApplication::quit, QKeySequence::Quit);

        QMenu* edit = menuBar()->addMenu("Edit");
        edit->addAction("Undo", QKeySequence::Undo);
        edit->addAction("Redo", QKeySequence::Redo);
        edit->addSeparator();

        QMenu* gizmoMenu = edit->addMenu("Gizmo Mode");
        gizmoMenu->addAction("Move",   this, &MainWindow::onGizmoMove,   QKeySequence("W"));
        gizmoMenu->addAction("Rotate", this, &MainWindow::onGizmoRotate, QKeySequence("E"));
        gizmoMenu->addAction("Scale",  this, &MainWindow::onGizmoScale,  QKeySequence("R"));

        QMenu* view = menuBar()->addMenu("View");
        view->addAction("Hierarchy",     [this]{ if (m_hierarchyTree) m_hierarchyTree->parentWidget()->parentWidget()->show(); });
        view->addAction("Inspector",     [this]{ if (m_inspectorArea) m_inspectorArea->parentWidget()->parentWidget()->show(); });
        view->addAction("Asset Browser", [this]{ if (m_assetList)     m_assetList->parentWidget()->parentWidget()->show(); });

        QMenu* help = menuBar()->addMenu("Help");
        help->addAction("About SabakaEngine", this, &MainWindow::onAbout);
    }

    void MainWindow::setupToolBar() {
        QToolBar* tb = addToolBar("Main");
        tb->setObjectName("mainToolBar");
        tb->setMovable(false);
        tb->setIconSize(QSize(16, 16));

        m_actPlay  = tb->addAction("▶  Play",  this, &MainWindow::onPlay);
        m_actPause = tb->addAction("⏸  Pause", this, &MainWindow::onPause);
        m_actStop  = tb->addAction("⏹  Stop",  this, &MainWindow::onStop);

        m_actPause->setEnabled(false);
        m_actStop->setEnabled(false);

        tb->addSeparator();

        m_actMove   = tb->addAction("Move",   this, &MainWindow::onGizmoMove);
        m_actRotate = tb->addAction("Rotate", this, &MainWindow::onGizmoRotate);
        m_actScale  = tb->addAction("Scale",  this, &MainWindow::onGizmoScale);

        m_actMove->setCheckable(true);
        m_actRotate->setCheckable(true);
        m_actScale->setCheckable(true);
        m_actMove->setChecked(true);

        tb->addSeparator();
        tb->addAction("Settings");
    }

    void MainWindow::setupDocks() {
        m_hierarchyTree = new QTreeWidget();
        m_hierarchyTree->setHeaderLabel("Scene");
        m_hierarchyTree->setAlternatingRowColors(true);
        m_hierarchyTree->addTopLevelItem(new QTreeWidgetItem(QStringList{"(empty scene)"}));
        makeDock("Hierarchy", m_hierarchyTree, Qt::LeftDockWidgetArea, 220, 300);

        auto* inspectorPlaceholder = new QLabel("Select an entity");
        inspectorPlaceholder->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        inspectorPlaceholder->setContentsMargins(8, 12, 8, 8);
        m_inspectorArea = new QScrollArea();
        m_inspectorArea->setWidget(inspectorPlaceholder);
        m_inspectorArea->setWidgetResizable(true);
        m_inspectorArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        makeDock("Inspector", m_inspectorArea, Qt::RightDockWidgetArea, 260, 300);

        m_assetList = new QListWidget();
        m_assetList->addItem("(no project loaded)");
        m_assetList->setViewMode(QListWidget::IconMode);
        m_assetList->setIconSize(QSize(48, 48));
        m_assetList->setResizeMode(QListWidget::Adjust);
        makeDock("Asset Browser", m_assetList, Qt::BottomDockWidgetArea, 0, 150);
    }

    void MainWindow::setupCentralViewport() {
        m_viewportWindow    = new ViewportWindow();
        m_viewportContainer = QWidget::createWindowContainer(m_viewportWindow, this);
        m_viewportContainer->setMinimumSize(320, 240);
        m_viewportContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_viewportContainer->setFocusPolicy(Qt::StrongFocus);
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

    void MainWindow::onNewScene() {
        m_hierarchyTree->clear();
        m_hierarchyTree->addTopLevelItem(new QTreeWidgetItem(QStringList{"(empty scene)"}));
        statusBar()->showMessage("New scene created");
    }

    void MainWindow::onOpenScene() {
        QString path = QFileDialog::getOpenFileName(
            this, "Open Scene", "", "Scene Files (*.scene *.json);;All Files (*)");
        if (!path.isEmpty())
            statusBar()->showMessage("Opened: " + path);
    }

    void MainWindow::onSaveScene() {
        QString path = QFileDialog::getSaveFileName(
            this, "Save Scene", "", "Scene Files (*.scene *.json);;All Files (*)");
        if (!path.isEmpty())
            statusBar()->showMessage("Saved: " + path);
    }

    void MainWindow::onPlay() {
        m_playing = true;
        m_actPlay->setEnabled(false);
        m_actPause->setEnabled(true);
        m_actStop->setEnabled(true);
        statusBar()->showMessage("Playing...");
    }

    void MainWindow::onPause() {
        m_playing = false;
        m_actPlay->setEnabled(true);
        m_actPause->setEnabled(false);
        statusBar()->showMessage("Paused");
    }

    void MainWindow::onStop() {
        m_playing = false;
        m_actPlay->setEnabled(true);
        m_actPause->setEnabled(false);
        m_actStop->setEnabled(false);
        statusBar()->showMessage("Stopped");
    }

    void MainWindow::onGizmoMove() {
        m_actMove->setChecked(true);
        m_actRotate->setChecked(false);
        m_actScale->setChecked(false);
        statusBar()->showMessage("Gizmo: Move (W)");
    }

    void MainWindow::onGizmoRotate() {
        m_actMove->setChecked(false);
        m_actRotate->setChecked(true);
        m_actScale->setChecked(false);
        statusBar()->showMessage("Gizmo: Rotate (E)");
    }

    void MainWindow::onGizmoScale() {
        m_actMove->setChecked(false);
        m_actRotate->setChecked(false);
        m_actScale->setChecked(true);
        statusBar()->showMessage("Gizmo: Scale (R)");
    }

    void MainWindow::onAbout() {
        QMessageBox::about(this, "SabakaEngine",
            "SabakaEngine Editor\n\nDirectX 11 | PhysX | XAudio2\nC++20");
    }

}