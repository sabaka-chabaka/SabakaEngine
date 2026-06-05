#include "editor/commands/CommandHistory.h"
#include "core/Logger.h"

namespace engine::editor {

    CommandHistory& CommandHistory::get() {
        static CommandHistory instance;
        return instance;
    }

    void CommandHistory::execute(std::unique_ptr<Command> cmd) {
        cmd->execute();
        LOG_INFO("[Undo] execute: " + cmd->description());
        m_undoStack.push_back(std::move(cmd));
        m_redoStack.clear();

        while (m_undoStack.size() > kMaxHistory)
            m_undoStack.pop_front();
    }

    void CommandHistory::undo() {
        if (m_undoStack.empty()) return;
        auto& cmd = m_undoStack.back();
        LOG_INFO("[Undo] undo: " + cmd->description());
        cmd->undo();
        m_redoStack.push_back(std::move(cmd));
        m_undoStack.pop_back();
    }

    void CommandHistory::redo() {
        if (m_redoStack.empty()) return;
        auto& cmd = m_redoStack.back();
        LOG_INFO("[Undo] redo: " + cmd->description());
        cmd->execute();
        m_undoStack.push_back(std::move(cmd));
        m_redoStack.pop_back();
    }

    std::string CommandHistory::undoDescription() const {
        return m_undoStack.empty() ? "" : m_undoStack.back()->description();
    }

    std::string CommandHistory::redoDescription() const {
        return m_redoStack.empty() ? "" : m_redoStack.back()->description();
    }

    void CommandHistory::clear() {
        m_undoStack.clear();
        m_redoStack.clear();
    }

}
