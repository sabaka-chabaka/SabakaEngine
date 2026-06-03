#pragma once
#include "editor/EditorCameraController.h"
#include <QWindow>
#include <QTimer>
#include <chrono>
#include <memory>

namespace engine::editor { class EditorApplication; }

namespace engine::editor {

    class ViewportWindow : public QWindow {
        Q_OBJECT
    public:
        explicit ViewportWindow(QWindow* parent = nullptr);
        ~ViewportWindow() override;

        EditorApplication* getEngine() const { return m_engine.get(); }

        signals:
            void engineReady(EditorApplication* engine);

    protected:
        void exposeEvent(QExposeEvent* event)       override;
        void resizeEvent(QResizeEvent* event)       override;
        void mousePressEvent(QMouseEvent* event)    override;
        void mouseReleaseEvent(QMouseEvent* event)  override;
        void mouseMoveEvent(QMouseEvent* event)     override;
        void keyPressEvent(QKeyEvent* event)        override;
        void keyReleaseEvent(QKeyEvent* event)      override;
        void wheelEvent(QWheelEvent* event)         override;

    private slots:
        void onTick();

    private:
        void startEngine();

        std::unique_ptr<EditorApplication>             m_engine;
        EditorCameraController                         m_camCtrl;
        QTimer                                         m_timer;
        std::chrono::high_resolution_clock::time_point m_lastTime;
        bool                                           m_engineReady = false;

        QPoint m_lastMousePos;
        bool   m_rightButtonDown = false;
    };

}