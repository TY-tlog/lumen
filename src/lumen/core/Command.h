#pragma once

#include <QString>

namespace lumen::core {

/// Abstract base class for undoable commands.
///
/// Each command knows how to execute itself and how to undo its effect.
/// Commands are managed by CommandBus which provides undo/redo stacks.
class Command {
public:
    virtual ~Command() = default;

    /// Apply the command's effect.
    virtual void execute() = 0;

    /// Reverse the command's effect.
    virtual void undo() = 0;

    /// Human-readable description (used for undo/redo menu items).
    [[nodiscard]] virtual QString description() const = 0;
};

}  // namespace lumen::core
