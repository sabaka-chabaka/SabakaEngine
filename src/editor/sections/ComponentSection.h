#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QFormLayout>
#include <QDoubleSpinBox>

namespace engine::editor {

    class ComponentSection : public QWidget {
        Q_OBJECT
    public:
        explicit ComponentSection(const QString& title, QWidget* parent = nullptr);

    protected:
        QFormLayout* formLayout() const { return m_form; }

        static QDoubleSpinBox* makeSpinBox(double min, double max,
                                           double step, int decimals,
                                           QWidget* parent);

        static QWidget* makeVec3Row(QFormLayout*    form,
                                    const QString&  label,
                                    QDoubleSpinBox*& outX,
                                    QDoubleSpinBox*& outY,
                                    QDoubleSpinBox*& outZ,
                                    QWidget*         parent);

    private slots:
        void onToggle();

    private:
        QWidget*     m_body     = nullptr;
        QToolButton* m_toggle   = nullptr;
        QFormLayout* m_form     = nullptr;
        bool         m_expanded = true;
    };

}
