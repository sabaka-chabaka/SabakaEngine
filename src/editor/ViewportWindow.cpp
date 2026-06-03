#define NOMINMAX
#include "editor/ViewportWindow.h"
#include "editor/EditorApplication.h"
#include "core/Logger.h"
#include <QResizeEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QCursor>
#include <windows.h>

using namespace std::chrono;

namespace engine::editor {

    ViewportWindow::ViewportWindow(QWindow* parent)
        : QWindow(parent)
    {
        setSurfaceType(QWindow::RasterSurface);
        setMinimumSize({ 320, 240 });
        connect(&m_timer, &QTimer::timeout, this, &ViewportWindow::onTick);
    }

    ViewportWindow::~ViewportWindow() {
        m_timer.stop();
    }

    void ViewportWindow::exposeEvent(QExposeEvent*) {
        if (isExposed() && !m_engineReady)
            startEngine();
    }

    void ViewportWindow::resizeEvent(QResizeEvent* event) {
        QWindow::resizeEvent(event);
        if (m_engine && m_engineReady) {
            int w = event->size().width();
            int h = std::max(event->size().height(), 1);
            m_engine->getDevice()->onResize(w, h);
        }
    }

    void ViewportWindow::startEngine() {
        HWND hwnd = reinterpret_cast<HWND>(winId());
        int  w    = std::max(width(),  1);
        int  h    = std::max(height(), 1);

        try {
            m_engine      = std::make_unique<EditorApplication>(hwnd, w, h);
            m_engineReady = true;
            m_lastTime    = high_resolution_clock::now();
            m_timer.start(0);
            m_camCtrl.setCamera(m_engine->getCamera());
            emit engineReady(m_engine.get());
            LOG_INFO("[ViewportWindow] engine started");
        }
        catch (const std::exception& e) {
            LOG_FATAL(std::string("[ViewportWindow] engine init failed: ") + e.what());
        }
    }

    void ViewportWindow::onTick() {
        if (!m_engineReady) return;
        auto  now       = high_resolution_clock::now();
        float deltaTime = duration<float>(now - m_lastTime).count();
        m_lastTime      = now;
        deltaTime       = std::min(deltaTime, 0.05f);
        m_camCtrl.update(deltaTime);
        m_engine->tick(deltaTime);
    }

    void ViewportWindow::mousePressEvent(QMouseEvent* event) {
        if (event->button() == Qt::RightButton) {
            m_rightButtonDown = true;
            m_lastMousePos    = event->globalPosition().toPoint();
            setCursor(Qt::BlankCursor);
            m_camCtrl.setActive(true);
            requestActivate();
        }
    }

    void ViewportWindow::mouseReleaseEvent(QMouseEvent* event) {
        if (event->button() == Qt::RightButton) {
            m_rightButtonDown = false;
            setCursor(Qt::ArrowCursor);
            m_camCtrl.setActive(false);
        }
    }

    void ViewportWindow::mouseMoveEvent(QMouseEvent* event) {
        if (!m_rightButtonDown) return;

        QPoint current = event->globalPosition().toPoint();
        int    dx      = current.x() - m_lastMousePos.x();
        int    dy      = current.y() - m_lastMousePos.y();

        if (dx == 0 && dy == 0) return;

        m_camCtrl.onMouseMove(dx, dy);

        QCursor::setPos(m_lastMousePos);
    }

    void ViewportWindow::keyPressEvent(QKeyEvent* event) {
        if (!event->isAutoRepeat())
            m_camCtrl.setKey(event->key(), true);
    }

    void ViewportWindow::keyReleaseEvent(QKeyEvent* event) {
        if (!event->isAutoRepeat())
            m_camCtrl.setKey(event->key(), false);
    }

    void ViewportWindow::wheelEvent(QWheelEvent* event) {
        if (!m_engineReady) return;
        float delta = static_cast<float>(event->angleDelta().y()) / 120.f;
        m_camCtrl.onScroll(delta);
        event->accept();
    }

}