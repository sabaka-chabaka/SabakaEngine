#pragma once
#include "editor/sections/ComponentSection.h"

namespace engine::core { class Transform; }

namespace engine::editor {

    class TransformSection : public ComponentSection {
        Q_OBJECT
    public:
        explicit TransformSection(core::Transform* transform, QWidget* parent = nullptr);

    private slots:
        void onPositionChanged();
        void onRotationChanged();
        void onScaleChanged();

    private:
        void refreshFromTransform();

        core::Transform* m_transform = nullptr;

        QDoubleSpinBox* m_posX = nullptr;
        QDoubleSpinBox* m_posY = nullptr;
        QDoubleSpinBox* m_posZ = nullptr;

        QDoubleSpinBox* m_rotX = nullptr;
        QDoubleSpinBox* m_rotY = nullptr;
        QDoubleSpinBox* m_rotZ = nullptr;

        QDoubleSpinBox* m_sclX = nullptr;
        QDoubleSpinBox* m_sclY = nullptr;
        QDoubleSpinBox* m_sclZ = nullptr;

        bool m_updating = false;
    };

}
