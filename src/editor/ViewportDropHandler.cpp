#include "editor/ViewportDropHandler.h"
#include "editor/EditorApplication.h"
#include "core/Logger.h"
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QUrl>

namespace engine::editor {

    static const QStringList kMeshExtensions   = { "mesh", "fbx", "obj", "gltf", "glb" };
    static const QStringList kTextureExtensions = { "png", "jpg", "jpeg", "tga", "dds" };

    ViewportDropHandler::ViewportDropHandler(QObject* parent)
        : QObject(parent) {}

    void ViewportDropHandler::setEngine(EditorApplication* engine) {
        m_engine = engine;
    }

    bool ViewportDropHandler::eventFilter(QObject* watched, QEvent* event) {
        switch (event->type()) {

        case QEvent::DragEnter: {
            auto* e = static_cast<QDragEnterEvent*>(event);
            if (e->mimeData()->hasFormat("application/x-sabaka-asset") ||
                e->mimeData()->hasUrls())
            {
                e->acceptProposedAction();
            }
            return true;
        }

        case QEvent::DragMove: {
            auto* e = static_cast<QDragMoveEvent*>(event);
            e->acceptProposedAction();
            return true;
        }

        case QEvent::Drop: {
            auto* e = static_cast<QDropEvent*>(event);
            QString path;

            if (e->mimeData()->hasFormat("application/x-sabaka-asset")) {
                path = QString::fromUtf8(
                    e->mimeData()->data("application/x-sabaka-asset"));
            } else if (!e->mimeData()->urls().isEmpty()) {
                path = e->mimeData()->urls().first().toLocalFile();
            }

            if (!path.isEmpty()) {
                handleDrop(path);
                e->acceptProposedAction();
            }
            return true;
        }

        default:
            return QObject::eventFilter(watched, event);
        }
    }

    void ViewportDropHandler::handleDrop(const QString& assetPath) {
        if (!m_engine) return;

        QFileInfo fi(assetPath);
        QString   ext = fi.suffix().toLower();
        QString   name = fi.baseName();

        if (kMeshExtensions.contains(ext)) {
            core::Entity* e = m_engine->createCube(name.toStdString());
            (void)e;
            LOG_INFO("[Drop] created cube entity from: " + assetPath.toStdString());
            emit entityDropped(m_engine, assetPath);
            return;
        }

        if (kTextureExtensions.contains(ext)) {
            core::Entity* e = m_engine->createEmpty(name.toStdString());
            (void)e;
            LOG_INFO("[Drop] created empty entity for texture: " + assetPath.toStdString());
            emit entityDropped(m_engine, assetPath);
            return;
        }

        core::Entity* e = m_engine->createEmpty(name.toStdString());
        (void)e;
        LOG_INFO("[Drop] created empty entity for asset: " + assetPath.toStdString());
        emit entityDropped(m_engine, assetPath);
    }

}