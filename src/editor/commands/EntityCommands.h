#pragma once
#include "editor/commands/Command.h"
#include <string>

namespace engine::editor { class EditorApplication; }

namespace engine::editor {

    class CreateEntityCommand : public Command {
    public:
        CreateEntityCommand(EditorApplication* app, std::string name, bool asCube);

        void execute() override;
        void undo()    override;
        std::string description() const override;

    private:
        EditorApplication* m_app;
        std::string        m_name;
        bool               m_asCube;
        uint64_t           m_createdId = 0;
    };

    class DestroyEntityCommand : public Command {
    public:
        DestroyEntityCommand(EditorApplication* app, uint64_t entityId, std::string snapshot);

        void execute() override;
        void undo()    override;
        std::string description() const override;

    private:
        EditorApplication* m_app;
        uint64_t           m_entityId;
        std::string        m_snapshot;
        std::string        m_name;
    };

}
