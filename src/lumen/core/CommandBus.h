#pragma once

#include "Command.h"

#include <QObject>
#include <QString>

#include <memory>
#include <vector>

namespace lumen::core {

/// Executes commands and manages undo/redo stacks.
///
/// All state-changing operations go through the CommandBus so they can
/// be undone and redone. See ADR-018 for design rationale.
class CommandBus : public QObject {
    Q_OBJECT

public:
    explicit CommandBus(QObject* parent = nullptr);
    ~CommandBus() override;

    /// Execute a command: calls cmd->execute(), pushes to undo stack,
    /// clears the redo stack.
    void execute(std::unique_ptr<Command> cmd);

    /// Undo the most recent command: pops from undo stack, calls
    /// cmd->undo(), pushes to redo stack.
    void undo();

    /// Redo the most recently undone command: pops from redo stack,
    /// calls cmd->execute(), pushes to undo stack.
    void redo();

    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;

    [[nodiscard]] QString undoDescription() const;
    [[nodiscard]] QString redoDescription() const;

signals:
    void commandExecuted(const QString& description);
    void undoRedoStateChanged();

private:
    std::vector<std::unique_ptr<Command>> undoStack_;
    std::vector<std::unique_ptr<Command>> redoStack_;
};

}  // namespace lumen::core
