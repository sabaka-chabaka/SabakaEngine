#define NOMINMAX
#include "editor/sections/TransformSection.h"
#include "core/Transform.h"
#include <DirectXMath.h>

namespace engine::editor {

    TransformSection::TransformSection(core::Transform* transform, QWidget* parent)
        : ComponentSection("Transform", parent)
        , m_transform(transform)
    {
        auto* form = formLayout();

        makeVec3Row(form, "Position", m_posX, m_posY, m_posZ, this);
        makeVec3Row(form, "Rotation", m_rotX, m_rotY, m_rotZ, this);
        makeVec3Row(form, "Scale",    m_sclX, m_sclY, m_sclZ, this);

        for (auto* sb : { m_posX, m_posY, m_posZ })
            connect(sb, &QDoubleSpinBox::valueChanged,
                    this, &TransformSection::onPositionChanged);

        for (auto* sb : { m_rotX, m_rotY, m_rotZ })
            connect(sb, &QDoubleSpinBox::valueChanged,
                    this, &TransformSection::onRotationChanged);

        for (auto* sb : { m_sclX, m_sclY, m_sclZ })
            connect(sb, &QDoubleSpinBox::valueChanged,
                    this, &TransformSection::onScaleChanged);

        refreshFromTransform();
    }

    void TransformSection::refreshFromTransform() {
        if (!m_transform) return;
        m_updating = true;

        auto pos = m_transform->getPosition();
        m_posX->setValue(pos.x);
        m_posY->setValue(pos.y);
        m_posZ->setValue(pos.z);

        auto rot = m_transform->getRotationEuler();
        m_rotX->setValue(DirectX::XMConvertToDegrees(rot.x));
        m_rotY->setValue(DirectX::XMConvertToDegrees(rot.y));
        m_rotZ->setValue(DirectX::XMConvertToDegrees(rot.z));

        auto scl = m_transform->getScale();
        m_sclX->setValue(scl.x);
        m_sclY->setValue(scl.y);
        m_sclZ->setValue(scl.z);

        m_updating = false;
    }

    void TransformSection::onPositionChanged() {
        if (m_updating || !m_transform) return;
        m_transform->setPosition(
            static_cast<float>(m_posX->value()),
            static_cast<float>(m_posY->value()),
            static_cast<float>(m_posZ->value()));
    }

    void TransformSection::onRotationChanged() {
        if (m_updating || !m_transform) return;
        m_transform->setRotationEuler(
            DirectX::XMConvertToRadians(static_cast<float>(m_rotX->value())),
            DirectX::XMConvertToRadians(static_cast<float>(m_rotY->value())),
            DirectX::XMConvertToRadians(static_cast<float>(m_rotZ->value())));
    }

    void TransformSection::onScaleChanged() {
        if (m_updating || !m_transform) return;
        m_transform->setScale(
            static_cast<float>(m_sclX->value()),
            static_cast<float>(m_sclY->value()),
            static_cast<float>(m_sclZ->value()));
    }

}
