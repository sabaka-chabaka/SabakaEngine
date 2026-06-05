#include "editor/commands/TransformCommands.h"
#include "core/Transform.h"

namespace engine::editor {

    MoveCommand::MoveCommand(core::Transform* t,
                             DirectX::XMFLOAT3 oldPos,
                             DirectX::XMFLOAT3 newPos,
                             std::string entityName)
        : m_transform(t)
        , m_oldPos(oldPos)
        , m_newPos(newPos)
        , m_name(std::move(entityName))
    {}

    void MoveCommand::execute() { m_transform->setPosition(m_newPos); }
    void MoveCommand::undo()    { m_transform->setPosition(m_oldPos); }
    std::string MoveCommand::description() const {
        return "Move " + m_name;
    }

    RotateCommand::RotateCommand(core::Transform* t,
                                 DirectX::XMFLOAT4 oldRot,
                                 DirectX::XMFLOAT4 newRot,
                                 std::string entityName)
        : m_transform(t)
        , m_oldRot(oldRot)
        , m_newRot(newRot)
        , m_name(std::move(entityName))
    {}

    void RotateCommand::execute() { m_transform->setRotationQuat(m_newRot); }
    void RotateCommand::undo()    { m_transform->setRotationQuat(m_oldRot); }
    std::string RotateCommand::description() const {
        return "Rotate " + m_name;
    }

    ScaleCommand::ScaleCommand(core::Transform* t,
                               DirectX::XMFLOAT3 oldScale,
                               DirectX::XMFLOAT3 newScale,
                               std::string entityName)
        : m_transform(t)
        , m_oldScale(oldScale)
        , m_newScale(newScale)
        , m_name(std::move(entityName))
    {}

    void ScaleCommand::execute() { m_transform->setScale(m_newScale); }
    void ScaleCommand::undo()    { m_transform->setScale(m_oldScale); }
    std::string ScaleCommand::description() const {
        return "Scale " + m_name;
    }

}
