#include "editor/AssetBrowserWidget.h"
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMimeData>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>
#include <QDrag>

namespace engine::editor {

    static const QStringList kAssetExtensions = {
        "*.mesh", "*.scene", "*.mat", "*.shader",
        "*.png",  "*.jpg",   "*.jpeg","*.tga", "*.dds",
        "*.wav",  "*.ogg",   "*.mp3",
        "*.fbx",  "*.obj",   "*.gltf", "*.glb"
    };

    class AssetFileProxy : public QSortFilterProxyModel {
    public:
        explicit AssetFileProxy(QObject* parent = nullptr)
            : QSortFilterProxyModel(parent) {}

        void setSearchText(const QString& text) {
            m_search = text.toLower();
            invalidateFilter();
        }

    protected:
        bool filterAcceptsRow(int row, const QModelIndex& parent) const override {
            auto* fs = static_cast<QFileSystemModel*>(sourceModel());
            QModelIndex idx = fs->index(row, 0, parent);

            if (fs->isDir(idx)) return false;

            QString name = fs->fileName(idx).toLower();
            if (!m_search.isEmpty() && !name.contains(m_search))
                return false;

            QString suffix = "*." + fs->fileInfo(idx).suffix().toLower();
            return kAssetExtensions.contains(suffix);
        }

    private:
        QString m_search;
    };

    class DragListView : public QListView {
    public:
        explicit DragListView(QWidget* parent = nullptr) : QListView(parent) {}

    protected:
        void startDrag(Qt::DropActions) override {
            QModelIndex idx = currentIndex();
            if (!idx.isValid()) return;

            auto* proxy = static_cast<QSortFilterProxyModel*>(model());
            auto* fs    = static_cast<QFileSystemModel*>(proxy->sourceModel());
            QString path = fs->filePath(proxy->mapToSource(idx));

            auto* mime = new QMimeData();
            mime->setData("application/x-sabaka-asset", path.toUtf8());
            mime->setUrls({ QUrl::fromLocalFile(path) });

            auto* drag = new QDrag(this);
            drag->setMimeData(mime);
            drag->exec(Qt::CopyAction);
        }
    };

    AssetBrowserWidget::AssetBrowserWidget(QWidget* parent)
        : QWidget(parent)
    {
        m_fsModel = new QFileSystemModel(this);
        m_fsModel->setRootPath(QDir::rootPath());
        m_fsModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot |
                             QDir::AllEntries | QDir::Files);
        m_fsModel->setNameFilterDisables(false);
        m_fsModel->setNameFilters(kAssetExtensions);

        m_fileProxy = new AssetFileProxy(this);
        m_fileProxy->setSourceModel(m_fsModel);

        auto* toolbar = new QToolBar(this);
        toolbar->setIconSize(QSize(16, 16));

        auto* rootBtn = new QPushButton("📂 Set Root", this);
        rootBtn->setFixedHeight(22);
        toolbar->addWidget(rootBtn);
        toolbar->addSeparator();

        m_searchBox = new QLineEdit(this);
        m_searchBox->setPlaceholderText("Search assets...");
        m_searchBox->setFixedHeight(22);
        m_searchBox->setClearButtonEnabled(true);
        toolbar->addWidget(m_searchBox);

        m_folderTree = new QTreeView(this);
        m_folderTree->setModel(m_fsModel);
        m_folderTree->setHeaderHidden(true);
        m_folderTree->hideColumn(1);
        m_folderTree->hideColumn(2);
        m_folderTree->hideColumn(3);
        m_folderTree->setMinimumWidth(140);
        m_folderTree->setMaximumWidth(260);



        m_fileGrid = new DragListView(this);
        m_fileGrid->setModel(m_fileProxy);
        m_fileGrid->setViewMode(QListView::IconMode);
        m_fileGrid->setIconSize(QSize(48, 48));
        m_fileGrid->setGridSize(QSize(80, 72));
        m_fileGrid->setResizeMode(QListView::Adjust);
        m_fileGrid->setDragEnabled(true);
        m_fileGrid->setDefaultDropAction(Qt::CopyAction);
        m_fileGrid->setSelectionMode(QAbstractItemView::SingleSelection);
        m_fileGrid->setWordWrap(true);
        m_fileGrid->setTextElideMode(Qt::ElideMiddle);

        auto* splitter = new QSplitter(Qt::Horizontal, this);
        splitter->addWidget(m_folderTree);
        splitter->addWidget(m_fileGrid);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        splitter->setSizes({ 180, 600 });

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(toolbar);
        layout->addWidget(splitter);

        connect(m_folderTree->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &AssetBrowserWidget::onFolderSelected);
        connect(rootBtn, &QPushButton::clicked,
                this, &AssetBrowserWidget::onSetRootClicked);
        connect(m_searchBox, &QLineEdit::textChanged,
                this, &AssetBrowserWidget::onSearchChanged);
        connect(m_fileGrid, &QListView::doubleClicked, this, [this](const QModelIndex& idx) {
            auto* fs   = static_cast<QFileSystemModel*>(m_fileProxy->sourceModel());
            QString path = fs->filePath(m_fileProxy->mapToSource(idx));
            emit assetDoubleClicked(path);
        });

        setRootPath(QDir::currentPath());
    }

    void AssetBrowserWidget::setRootPath(const QString& path) {
        QModelIndex root = m_fsModel->setRootPath(path);
        m_folderTree->setRootIndex(root);
        m_folderTree->setCurrentIndex(root);
        m_fileProxy->setFilterFixedString("");
        updateFileGrid(root);
    }

    QString AssetBrowserWidget::rootPath() const {
        return m_fsModel->rootPath();
    }

    void AssetBrowserWidget::updateFileGrid(const QModelIndex& fsIndex) {
        QModelIndex proxyIdx = m_fileProxy->mapFromSource(fsIndex);
        m_fileGrid->setRootIndex(proxyIdx);
    }

    void AssetBrowserWidget::onFolderSelected(const QModelIndex& current, const QModelIndex&) {
        if (!current.isValid()) return;
        if (!m_fsModel->isDir(current)) return;
        updateFileGrid(current);
    }

    void AssetBrowserWidget::onSetRootClicked() {
        QString dir = QFileDialog::getExistingDirectory(
            this, "Select Project Root", m_fsModel->rootPath());
        if (!dir.isEmpty())
            setRootPath(dir);
    }

    void AssetBrowserWidget::onSearchChanged(const QString& text) {
        static_cast<AssetFileProxy*>(m_fileProxy)->setSearchText(text);
    }

}