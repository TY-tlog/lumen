# ADR-023: Inline title editor as PlotCanvas overlay

## Status
Accepted (Phase 3b)

## Context
Phase 3b adds title editing. The title is a text element positioned
above the plot area. The editing UX must feel natural to scientists
familiar with MATLAB, Prism, and PowerPoint-style figure editing:
click the title, type to edit, press Enter to confirm.

## Decision
Implement the title editor as a QLineEdit overlay positioned
absolutely within PlotCanvas at the title rectangle coordinates:

1. Double-click on the title area (detected by HitTester
   hitNonSeriesElement returning HitKind::Title).
2. PlotCanvas creates a QLineEdit child widget positioned at the
   title rect (from PlotScene layout), matching the title font
   (titleFontPx, titleWeight).
3. QLineEdit is frameless (no border) with transparent background,
   blending into the plot.
4. Pre-filled with current title text. If no title exists (empty
   area above plot), the editor starts empty.
5. Enter key → create ChangeTitleCommand via CommandBus, hide
   editor.
6. Escape key → hide editor, no command.
7. Focus lost (click elsewhere) → apply (same as Enter).

InteractionController enters a new mode EditingTitleInline while
the editor is visible. In this mode, mouse events (pan, zoom) are
suppressed to prevent accidental interaction while typing.

Font size and weight editing is accessed via a separate TitleDialog
opened by right-click on the title area. This keeps the inline
editor simple (text only) and the TitleDialog focused on
formatting.

## Consequences
- + Matches MATLAB/Prism title editing UX exactly
- + Zero-click overhead: double-click → type → Enter
- + Works even when no title exists (creates title from empty)
- + Lightweight: one QLineEdit, no separate window
- - Positioning requires knowing the title rect precisely, which
  depends on dynamic margins (T6). If margins change while editing
  (e.g., another property dialog is open), the editor may shift.
  Mitigated by debounce in computeMargins.
- - QLineEdit overlay on a custom-painted QWidget requires manual
  positioning (not automatic layout). Acceptable for one widget.

## Alternatives considered
- **TitleDialog only (no inline)**: a QDialog with a text field
  for the title. Works but feels heavy for editing one line of
  text. Every other scientific plotting tool supports direct title
  editing. Rejected for UX reasons.
- **Separate floating window**: a small frameless window positioned
  near the title. More complex (window management, focus handling,
  Wayland compatibility) for the same result. Rejected; QLineEdit
  as child widget is simpler and sufficient.
