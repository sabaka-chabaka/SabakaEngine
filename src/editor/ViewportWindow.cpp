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
        setSurfaceType(QWindow::OpenGLSurface);
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
            int h = event->size().height();
            m_engine->getDevice()->onResize(w, h);
        }
    }

    void ViewportWindow::startEngine() {
        HWND hwnd = reinterpret_cast<HWND>(winId());
        int  w    = width();
        int  h    = height() > 0 ? height() : 1;

        try {
            m_engine      = std::make_unique<EditorApplication>(hwnd, w, h);
            m_engineReady = true;
            m_lastTime    = high_resolution_clock::now();
            m_timer.start(0);
            LOG_INFO("[ViewportWindow] engine started on HWND " + std::to_string(reinterpret_cast<uintptr_t>(hwnd)));
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