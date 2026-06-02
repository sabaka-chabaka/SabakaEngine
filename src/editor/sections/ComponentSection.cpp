#include "editor/sections/ComponentSection.h"

namespace engine::editor {

    ComponentSection::ComponentSection(const QString& title, QWidget* parent)
        : QWidget(parent)
    {
        auto* outer = new QVBoxLayout(this);
        outer->setContentsMargins(0, 0, 0, 2);
        outer->setSpacing(0);

        auto* header = new QWidget();
        header->setFixedHeight(24);
        header->setStyleSheet(
            "QWidget { background: palette(mid); border-radius: 3px; }");

        auto* hl = new QHBoxLayout(header);
        hl->setContentsMargins(6, 0, 4, 0);
        hl->setSpacing(4);

        m_toggle = new QToolButton();
        m_toggle->setText("▼");
        m_toggle->setFixedSize(18, 18);
        m_toggle->setStyleSheet("QToolButton { border: none; background: transparent; }");
        connect(m_toggle, &QToolButton::clicked, this, &ComponentSection::onToggle);

        auto* lbl = new QLabel(title);
        lbl->setStyleSheet("font-weight: bold; background: transparent;");

        hl->addWidget(m_toggle);
        hl->addWidget(lbl);
        hl->addStretch();

        outer->addWidget(header);

        m_body = new QWidget();
        m_body->setStyleSheet(
            "QWidget { background: palette(base); border: 1px solid palette(mid);"
            "border-top: none; border-radius: 0 0 3px 3px; }");

        auto* bodyLayout = new QVBoxLayout(m_body);
        bodyLayout->setContentsMargins(8, 6, 8, 8);

        m_form = new QFormLayout();
        m_form->setContentsMargins(0, 0, 0, 0);
        m_form->setRowWrapPolicy(QFormLayout::DontWrapRows);
        m_form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_form->setHorizontalSpacing(8);
        m_form->setVerticalSpacing(4);

        bodyLayout->addLayout(m_form);
        outer->addWidget(m_body);
    }

    void ComponentSection::onToggle() {
        m_expanded = !m_expanded;
        m_body->setVisible(m_expanded);
        m_toggle->setText(m_expanded ? "▼" : "▶");
    }

    QDoubleSpinBox* ComponentSection::makeSpinBox(double min, double max,
                                                   double step, int decimals,
                                                   QWidget* parent)
    {
        auto* sb = new QDoubleSpinBox(parent);
        sb->setRange(min, max);
        sb->setSingleStep(step);
        sb->setDecimals(decimals);
        sb->setFixedWidth(70);
        sb->setButtonSymbols(QAbstractSpinBox::NoButtons);
        sb->setAlignment(Qt::AlignRight);
        return sb;
    }

    QWidget* ComponentSection::makeVec3Row(QFormLayout*    form,
                                           const QString&  label,
                                           QDoubleSpinBox*& outX,
                                           QDoubleSpinBox*& outY,
                                           QDoubleSpinBox*& outZ,
                                           QWidget*         parent)
    {
        auto* row = new QWidget(parent);
        auto* hl  = new QHBoxLayout(row);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(4);

        outX = makeSpinBox(-1e6, 1e6, 0.1, 3, parent);
        outY = makeSpinBox(-1e6, 1e6, 0.1, 3, parent);
        outZ = makeSpinBox(-1e6, 1e6, 0.1, 3, parent);

        auto makeAxisLabel = [&](const QString& text, const char* color) {
            auto* lbl = new QLabel(text, parent);
            lbl->setStyleSheet(
                QString("color: %1; font-weight: bold; font-size: 10px;").arg(color));
            lbl->setFixedWidth(10);
            return lbl;
        };

        hl->addWidget(makeAxisLabel("X", "#e05555"));
        hl->addWidget(outX);
        hl->addWidget(makeAxisLabel("Y", "#55aa55"));
        hl->addWidget(outY);
        hl->addWidget(makeAxisLabel("Z", "#5599e0"));
        hl->addWidget(outZ);

        form->addRow(label, row);
        return row;
    }

}
