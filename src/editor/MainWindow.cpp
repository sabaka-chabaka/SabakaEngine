#include "editor/MainWindow.h"
#include "editor/ViewportWindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QTreeWidgetItem>

namespace engine::editor {

    MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
    {
        setWindowTitle("SabakaEngine Editor");
        resize(1600, 900);

        setupMenuBar();
        setupToolBar();
        setupDocks();
        setupCentralViewport();

        statusBar()->showMessage("Ready");
    }

    void MainWindow::setupMenuBar() {
        QMenu* fileMenu = menuBar()->addMenu("File");
        fileMenu->addAction("New Scene",  QKeySequence::New);
        fileMenu->addAction("Open Scene", QKeySequence::Open);
        fileMenu->addAction("Save Scene", QKeySequence::Save);
        fileMenu->addSeparator();
        fileMenu->addAction("Exit", QApplication::instance(), &QApplication::quit,
                            QKeySequence::Quit);

        QMenu* editMenu = menuBar()->addMenu("Edit");
        editMenu->addAction("Undo", QKeySequence::Undo);
        editMenu->addAction("Redo", QKeySequence::Redo);

        QMenu* viewMenu = menuBar()->addMenu("View");
        viewMenu->addAction("Reset Layout");

        menuBar()->addMenu("Help")->addAction("About SabakaEngine");
    }

    void MainWindow::setupToolBar() {
        QToolBar* tb = addToolBar("Main");
        tb->setMovable(false);
        tb->addAction("▶  Play");
        tb->addAction("⏸  Pause");
        tb->addAction("⏹  Stop");
        tb->addSeparator();
        tb->addAction("Move");
        tb->addAction("Rotate");
        tb->addAction("Scale");
    }

    void MainWindow::setupDocks() {
        m_hierarchyTree = new QTreeWidget();
        m_hierarchyTree->setHeaderLabel("Scene Hierarchy");
        m_hierarchyTree->addTopLevelItem(new QTreeWidgetItem(QStringList{"(empty scene)"}));
        makeDock("Hierarchy", m_hierarchyTree, Qt::LeftDockWidgetArea);

        auto* inspectorPlaceholder = new QLabel("Select an entity to inspect");
        inspectorPlaceholder->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        inspectorPlaceholder->setMargin(12);
        m_inspectorArea = new QScrollArea();
        m_inspectorArea->setWidget(inspectorPlaceholder);
        m_inspectorArea->setWidgetResizable(true);
        makeDock("Inspector", m_inspectorArea, Qt::RightDockWidgetArea);

        m_assetList = new QListWidget();
        m_assetList->addItem("(no project loaded)");
        makeDock("Asset Browser", m_assetList, Qt::BottomDockWidgetArea);
    }

    void MainWindow::setupCentralViewport() {
        m_viewportWindow    = new ViewportWindow();
        m_viewportContainer = QWidget::createWindowContainer(m_viewportWindow, this);
        m_viewportContainer->setMinimumSize(320, 240);
        m_viewportContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setCentralWidget(m_viewportContainer);
    }

    QDockWidget* MainWindow::makeDock(const QString& title, QWidget* contents,
                                      Qt::DockWidgetArea area) {
        auto* dock = new QDockWidget(title, this);
        dock->setWidget(contents);
        dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        addDockWidget(area, dock);
        return dock;
    }

}