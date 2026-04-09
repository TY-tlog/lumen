#include "CommandBus.h"

namespace lumen::core {

CommandBus::CommandBus(QObject* parent)
    : QObject(parent) {}

CommandBus::~CommandBus() = default;

void CommandBus::execute(std::unique_ptr<Command> cmd) {
    cmd->execute();
    const QString desc = cmd->description();
    undoStack_.push_back(std::move(cmd));
    redoStack_.clear();
    emit commandExecuted(desc);
    emit undoRedoStateChanged();
}

void CommandBus::undo() {
    if (undoStack_.empty()) {
        return;
    }
    auto cmd = std::move(undoStack_.back());
    undoStack_.pop_back();
    cmd->undo();
    redoStack_.push_back(std::move(cmd));
    emit undoRedoStateChanged();
}

void CommandBus::redo() {
    if (redoStack_.empty()) {
        return;
    }
    auto cmd = std::move(redoStack_.back());
    redoStack_.pop_back();
    cmd->execute();
    undoStack_.push_back(std::move(cmd));
    emit undoRedoStateChanged();
}

bool CommandBus::canUndo() const {
    return !undoStack_.empty();
}

bool CommandBus::canRedo() const {
    return !redoStack_.empty();
}

QString CommandBus::undoDescription() const {
    if (undoStack_.empty()) {
        return {};
    }
    return undoStack_.back()->description();
}

QString CommandBus::redoDescription() const {
    if (redoStack_.empty()) {
        return {};
    }
    return redoStack_.back()->description();
}

}  // namespace lumen::core
