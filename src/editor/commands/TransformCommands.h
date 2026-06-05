#pragma once
#include "editor/commands/Command.h"
#include <DirectXMath.h>
#include <string>

namespace engine::core { class Transform; }

namespace engine::editor {

    class MoveCommand : public Command {
    public:
        MoveCommand(core::Transform* t,
                    DirectX::XMFLOAT3 oldPos,
                    DirectX::XMFLOAT3 newPos,
                    std::string entityName);

        void execute() override;
        void undo()    override;
        std::string description() const override;

    private:
        core::Transform*  m_transform;
        DirectX::XMFLOAT3 m_oldPos;
        DirectX::XMFLOAT3 m_newPos;
        std::string       m_name;
    };

    class RotateCommand : public Command {
    public:
        RotateCommand(core::Transform* t,
                      DirectX::XMFLOAT4 oldRot,
                      DirectX::XMFLOAT4 newRot,
                      std::string entityName);

        void execute() override;
        void undo()    override;
        std::string description() const override;

    private:
        core::Transform*  m_transform;
        DirectX::XMFLOAT4 m_oldRot;
        DirectX::XMFLOAT4 m_newRot;
        std::string       m_name;
    };

    class ScaleCommand : public Command {
    public:
        ScaleCommand(core::Transform* t,
                     DirectX::XMFLOAT3 oldScale,
                     DirectX::XMFLOAT3 newScale,
                     std::string entityName);

        void execute() override;
        void undo()    override;
        std::string description() const override;

    private:
        core::Transform*  m_transform;
        DirectX::XMFLOAT3 m_oldScale;
        DirectX::XMFLOAT3 m_newScale;
        std::string       m_name;
    };

}
