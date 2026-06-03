#define NOMINMAX
#include "editor/ViewportWindow.h"
#include "editor/EditorApplication.h"
#include "core/Logger.h"
#include "platform/Input.h"
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

        engine::platform::InputSystem::get().endFrame();
    }

    void ViewportWindow::mousePressEvent(QMouseEvent* event) {
        auto& input = engine::platform::InputSystem::get();
        if (event->button() == Qt::LeftButton)  input.injectMouseDown(engine::platform::MouseButton::Left);
        if (event->button() == Qt::RightButton) {
            input.injectMouseDown(engine::platform::MouseButton::Right);
            m_rightButtonDown = true;
            m_lastMousePos    = event->globalPosition().toPoint();
            setCursor(Qt::BlankCursor);
            m_camCtrl.setActive(true);
            requestActivate();
            setKeyboardGrabEnabled(true);
            emit rightMouseButtonChanged(true);
        }
        if (event->button() == Qt::MiddleButton) input.injectMouseDown(engine::platform::MouseButton::Middle);
    }

    void ViewportWindow::mouseReleaseEvent(QMouseEvent* event) {
        auto& input = engine::platform::InputSystem::get();
        if (event->button() == Qt::LeftButton)  input.injectMouseUp(engine::platform::MouseButton::Left);
        if (event->button() == Qt::RightButton) {
            input.injectMouseUp(engine::platform::MouseButton::Right);
            m_rightButtonDown = false;
            setCursor(Qt::ArrowCursor);
            m_camCtrl.setActive(false);
            setKeyboardGrabEnabled(false);
            // release all keys to avoid stuck movement after releasing RMB
            using engine::platform::Key;
            for (Key k : { Key::W, Key::A, Key::S, Key::D, Key::Q, Key::E, Key::Shift, Key::Control }) {
                input.injectKeyUp(k);
            }
            emit rightMouseButtonChanged(false);
        }
        if (event->button() == Qt::MiddleButton) input.injectMouseUp(engine::platform::MouseButton::Middle);
    }

    void ViewportWindow::mouseMoveEvent(QMouseEvent* event) {
        auto& input = engine::platform::InputSystem::get();
        input.injectMousePosition(static_cast<float>(event->position().x()), static_cast<float>(event->position().y()));

        if (!m_rightButtonDown) return;

        // Use globalPosition for movement calculation to avoid issues with cursor jumps
        QPoint current = event->globalPosition().toPoint();
        int    dx      = current.x() - m_lastMousePos.x();
        int    dy      = current.y() - m_lastMousePos.y();

        if (dx == 0 && dy == 0) return;

        input.injectMouseDelta(static_cast<float>(dx), static_cast<float>(dy));
        
        // Lock cursor to the original position
        QCursor::setPos(m_lastMousePos);
    }

    static engine::platform::Key qtKeyToEngine(int key) {
        using namespace engine::platform;
        // Qt upper cases keys by default, so 'A' is 0x41
        if (key >= Qt::Key_A && key <= Qt::Key_Z) return static_cast<Key>(key);
        if (key >= 'a' && key <= 'z') return static_cast<Key>(key - 'a' + 'A');
        if (key >= Qt::Key_0 && key <= Qt::Key_9) return static_cast<Key>(key);
        switch (key) {
            case Qt::Key_Space:    return Key::Space;
            case Qt::Key_Return:   return Key::Enter;
            case Qt::Key_Escape:   return Key::Escape;
            case Qt::Key_Tab:      return Key::Tab;
            case Qt::Key_Backspace:return Key::Backspace;
            case Qt::Key_Delete:   return Key::Delete;
            case Qt::Key_Left:     return Key::Left;
            case Qt::Key_Right:    return Key::Right;
            case Qt::Key_Up:       return Key::Up;
            case Qt::Key_Down:     return Key::Down;
            case Qt::Key_Shift:    return Key::Shift;
            case Qt::Key_Control:  return Key::Control;
            case Qt::Key_Alt:      return Key::Alt;
            default:               return Key::None;
        }
    }

    void ViewportWindow::keyPressEvent(QKeyEvent* event) {
        if (!event->isAutoRepeat()) {
            engine::platform::Key key = qtKeyToEngine(event->key());
            engine::platform::InputSystem::get().injectKeyDown(key);
        }
        
        if (m_rightButtonDown) {
            event->accept();
        }
        else {
            event->ignore();
        }
    }

    void ViewportWindow::keyReleaseEvent(QKeyEvent* event) {
        if (!event->isAutoRepeat()) {
            engine::platform::InputSystem::get().injectKeyUp(qtKeyToEngine(event->key()));
        }
        
        if (m_rightButtonDown) {
            event->accept();
        }
        else {
            event->ignore();
        }
    }

    void ViewportWindow::wheelEvent(QWheelEvent* event) {
        if (!m_engineReady) return;
        float delta = static_cast<float>(event->angleDelta().y()) / 120.f;
        m_camCtrl.onScroll(delta);
        event->accept();
    }

}