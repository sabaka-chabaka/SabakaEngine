#pragma once
#include <QWidget>
#include <QFileSystemModel>

class QTreeView;
class QListView;
class QLineEdit;
class QSortFilterProxyModel;

namespace engine::editor {

    class AssetBrowserWidget : public QWidget {
        Q_OBJECT
    public:
        explicit AssetBrowserWidget(QWidget* parent = nullptr);

        void setRootPath(const QString& path);
        QString rootPath() const;

        signals:
            void assetDoubleClicked(const QString& absolutePath);

    private slots:
        void onFolderSelected(const QModelIndex& current, const QModelIndex& previous);
        void onSetRootClicked();
        void onSearchChanged(const QString& text);

    private:
        void updateFileGrid(const QModelIndex& fsIndex);

        QFileSystemModel*      m_fsModel      = nullptr;
        QSortFilterProxyModel* m_fileProxy    = nullptr;
        QTreeView*             m_folderTree   = nullptr;
        QListView*             m_fileGrid     = nullptr;
        QLineEdit*             m_searchBox    = nullptr;
    };

}