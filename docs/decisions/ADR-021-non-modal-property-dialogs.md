# ADR-021: Non-modal property dialogs for all editable elements

## Status
Accepted (Phase 3b); pattern established in Phase 3a with
LinePropertyDialog.

## Context
Lumen's editing model is: double-click a plot element, a dialog
opens, the user edits properties, OK applies via CommandBus. Phase
3a shipped LinePropertyDialog (modal QDialog). Phase 3b adds three
more dialogs: AxisDialog, TitleDialog, LegendDialog.

The question is whether these should be modal (blocks the window),
non-modal (floating, user can interact with the plot while dialog
is open), or a dockable inspector panel (persistent, reflects
whatever element is selected).

## Decision
Use non-modal QDialog for all four property dialogs. Each dialog:

- Opens on double-click (or right-click for TitleDialog font
  options)
- Floats above the plot, does not block interaction
- Shows current properties of the clicked element
- OK creates a Command and executes via CommandBus
- Cancel dismisses with no Command created
- Closing the dialog is equivalent to Cancel

The inline title editor (QLineEdit overlay on double-click) is a
special case for the title element — it is not a dialog but a
transient overlay. TitleDialog (for font size/weight) is accessed
via right-click.

All dialogs follow the same API pattern:
```cpp
class XxxDialog : public QDialog {
    void setProperties(/* current values */);
    XxxProperties result() const;
};
```

## Consequences
- + User can pan/zoom while dialog is open (compare values visually)
- + Consistent pattern across all four element types
- + Simple implementation: standard QDialog with QFormLayout
- + Modal blocking avoided: user workflow is not interrupted
- - Non-modal dialogs can become orphaned if user switches documents
  (acceptable for Phase 3b; Phase 4 can track open dialogs)
- - Multiple dialogs can be open simultaneously (e.g., AxisDialog
  and LegendDialog). Acceptable but slightly cluttered.

## Alternatives considered
- **Modal dialogs**: simpler to implement (no orphan problem), but
  blocks the user from seeing the plot while editing. MATLAB uses
  non-modal, so this conflicts with the target UX.
- **Dockable inspector panel**: one persistent panel that reflects
  whatever element is selected (like Adobe Illustrator's Properties
  panel). Technically superior but significantly more complex:
  requires selection state management, panel content switching,
  multi-element selection. Deferred to Phase 5 when the property
  inspector is built as a dedicated feature.
