#pragma once
#include "editor/commands/Command.h"
#include <deque>
#include <memory>

namespace engine::editor {

    class CommandHistory {
    public:
        static CommandHistory& get();

        void execute(std::unique_ptr<Command> cmd);
        void undo();
        void redo();

        bool canUndo() const { return !m_undoStack.empty(); }
        bool canRedo() const { return !m_redoStack.empty(); }

        std::string undoDescription() const;
        std::string redoDescription() const;

        void clear();

        static constexpr size_t kMaxHistory = 100;

    private:
        CommandHistory() = default;

        std::deque<std::unique_ptr<Command>> m_undoStack;
        std::deque<std::unique_ptr<Command>> m_redoStack;
    };

}
