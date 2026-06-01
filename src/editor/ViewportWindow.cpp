#define NOMINMAX
#include "editor/ViewportWindow.h"
#include "editor/EditorApplication.h"
#include "core/Logger.h"
#include <QResizeEvent>
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

        deltaTime = std::min(deltaTime, 0.05f);

        m_engine->tick(deltaTime);
    }

}