#pragma once
#include <QObject>

namespace engine::editor {
    class EditorApplication;
}

namespace engine::editor {

    class ViewportDropHandler : public QObject {
        Q_OBJECT
    public:
        explicit ViewportDropHandler(QObject* parent = nullptr);

        void setEngine(EditorApplication* engine);

        signals:
            void entityDropped(engine::editor::EditorApplication* engine,
                               const QString& assetPath);

    protected:
        bool eventFilter(QObject* watched, QEvent* event) override;

    private:
        void handleDrop(const QString& assetPath);

        EditorApplication* m_engine = nullptr;
    };

}