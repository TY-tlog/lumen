# ADR-029: Series type is immutable after creation

## Status
Accepted (Phase 5)

## Context
Users pick a plot type (Line/Scatter/Bar) when adding a series via
the column picker. The question is whether the type can be changed
after creation (e.g., via a property dialog or context menu).

## Decision
Series type is immutable after creation. To change a series from
Line to Scatter, the user must remove it and add a new one with
the desired type. The column picker "Plot type" combo sets the type
at creation time.

## Consequences
- + Simple: each PlotItem subclass has fixed identity
- + No complex state migration (e.g., what happens to line width
  when converting to bar? What happens to marker shape when
  converting to line?)
- + Property dialogs are type-specific without a "Type" dropdown
  that triggers widget replacement
- + CommandBus doesn't need a "ChangeSeriesType" command with
  complex undo logic
- - User must delete and re-add to change type, losing custom
  properties. Acceptable because type changes are rare.
- - No "convert to scatter" convenience. Acceptable for Phase 5.

## Alternatives considered
- **Mutable type via dialog**: add a "Type" dropdown to each
  property dialog that triggers conversion. Rejected: complex
  state migration. Line width has no meaning for bars. Marker
  shape has no meaning for lines. Each conversion would need
  special-case logic.
- **Automatic type inference from data**: detect whether data
  looks like a scatter (unordered X) or line (monotonic X) and
  set the type automatically. Rejected: takes control from the
  user. The same data might be plotted as any type depending on
  the user's intent.
