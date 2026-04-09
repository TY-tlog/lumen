# ADR-022: Dynamic plot margins computed from content

## Status
Accepted (Phase 3b). **Resolves ADR-013** (hardcoded plot margins
60/50/30/15 as tech debt, accepted in Phase 2).

## Context
ADR-013 documented that PlotScene::computePlotArea() uses fixed
pixel margins (left=60, bottom=50, top=30/15, right=15) that
work for the reference electrophysiology data but break for long
axis labels, missing titles, or OutsideRight legend placement.
ADR-013 targeted Phase 4 for the refactor.

Phase 3b introduces axis label editing, title editing, and legend
position editing. All three directly affect how much margin space
is needed. Implementing dynamic margins is no longer optional — it
is a prerequisite for correct layout after edits.

## Decision
Replace the hardcoded constants in PlotScene with a
computeMargins() method that measures actual content:

```cpp
struct PlotMargins { double left, top, right, bottom; };
PlotMargins computeMargins(const QFontMetrics& tickFm,
                           const QFontMetrics& labelFm,
                           const QFontMetrics& titleFm) const;
```

Computation:
- **left**: max Y tick label width (horizontalAdvance on formatted
  tick strings) + tokens::spacing::md + Y axis label rotated
  height + tokens::spacing::sm
- **bottom**: X tick label height + tokens::spacing::md + X axis
  label height + tokens::spacing::sm
- **top**: if title set → titleFm.height() + tokens::spacing::md;
  else tokens::spacing::sm
- **right**: if legend.position == OutsideRight → legend width +
  tokens::spacing::md; else tokens::spacing::md

computePlotArea() calls computeMargins() on every paint. To
prevent margin oscillation during live edits (e.g., rapidly
changing axis labels), a 1-pixel debounce threshold is applied:
if new margins differ from cached margins by ≤1px in all
dimensions, the cached values are reused.

All spacing values come from tokens::spacing (ADR-008 design
system), not literal pixel values. This also resolves the
secondary concern in ADR-013 about violating the "no hardcoded
sizes" rule.

## Consequences
- + Margins adapt to content: long labels get more space
- + Title add/remove correctly adjusts top margin
- + OutsideRight legend gets dedicated right margin
- + Spacing uses DesignTokens, consistent with Apple-mood system
- + Resolves ADR-013 completely
- - Requires QFontMetrics at paint time (cheap: ~0.1ms per call)
- - Debounce adds slight visual delay (1 frame, 16ms) before
  margins stabilize after a change

## Alternatives considered
- **User-configurable margins**: let the user set margins via a
  dialog. Rejected: the user shouldn't need to manually adjust
  margins — the system should compute them correctly. User
  override can be added in Phase 5 if needed.
- **Content-aware fixed presets**: e.g., "small labels" = 50px,
  "large labels" = 80px. Rejected: still hardcoded, just with
  more options. Dynamic computation is the correct solution.
- **Keep hardcoded margins** (ADR-013 status quo): rejected; Phase
  3b axis label editing makes this untenable. Editing a label to
  "Membrane potential (mV)" would clip with 60px left margin.
