# ADR-018: CommandBus with undo/redo stack

## Status
Accepted (Phase 3a); implements ADR-006's design.

## Context
ADR-006 (Phase 0) established that all state-changing operations
go through a CommandBus with undo/redo. Phase 3a introduces the
first user-initiated state change: editing a line series's visual
properties (color, width, style, name, visibility). This is the
trigger to implement the CommandBus.

The CommandBus must support:
1. Execute a command (applies the change, pushes to undo stack).
2. Undo (pops from undo stack, pushes to redo stack).
3. Redo (pops from redo stack, pushes to undo stack).
4. Executing a new command clears the redo stack.
5. Signals when undo/redo state changes (for menu item enable/
   disable).

## Decision
Implement CommandBus as a QObject owning two stacks of
`std::unique_ptr<Command>`:

```cpp
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    [[nodiscard]] virtual QString description() const = 0;
};

class CommandBus : public QObject {
    Q_OBJECT
public:
    void execute(std::unique_ptr<Command> cmd);
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
signals:
    void commandExecuted(const QString& description);
    void undoRedoStateChanged();
private:
    std::vector<std::unique_ptr<Command>> undoStack_;
    std::vector<std::unique_ptr<Command>> redoStack_;
};
```

The first concrete command is `ChangeLineStyleCommand`:
- Captures: PlotScene pointer, series index, old style+name+
  visibility, new style+name+visibility.
- `execute()` applies new values via LineSeries setters.
- `undo()` restores old values.

CommandBus is owned by Application, passed through MainWindow to
PlotCanvasDock. All UI edit operations create a Command and submit
it to the bus.

## Consequences
- + Free undo/redo for every edit operation
- + Reproducible action log (future: session replay)
- + Same surface for human edits and future scripted/agent edits
- + Clean separation: UI creates Commands, CommandBus manages state
- - Every edit requires a Command subclass (~30 lines each)
- - Phase 3a has only one command type; the infrastructure pays off
  starting Phase 4 when axis editing, annotations, etc. are added
- - No command merging (e.g., rapid color changes create many undo
  entries). Acceptable for now; Phase 5 can add merge logic.

## Alternatives considered
- **Direct mutation without undo**: rejected; ADR-006 mandates
  CommandBus, and undo is a core UX expectation for a scientific
  tool where the user experiments with visual settings.
- **Qt's QUndoStack / QUndoCommand**: Qt provides a built-in undo
  framework. Rejected because: (a) QUndoCommand requires inheriting
  from QUndoCommand (no multiple inheritance with our Command base),
  (b) QUndoStack has opinionated view integration (QUndoView) we
  don't need yet, (c) our Command interface is simpler and matches
  CLAUDE.md's preference for self-built core abstractions. If we
  need QUndoView in Phase 5, we can wrap our Commands in
  QUndoCommand adapters.
- **Event sourcing**: store events instead of commands. More
  powerful but significantly more complex. Overkill for visual
  property edits. Deferred indefinitely.
