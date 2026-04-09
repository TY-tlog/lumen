# ADR-013: Plot area margins are hardcoded (known tech debt)

## Status
Accepted (Phase 2); refactor targeted for Phase 4.
**To be resolved in Phase 3b** (see ADR-022 and Task T6 in
docs/plans/phase-3b-plan.md). PlotScene::computeMargins() will
replace hardcoded 60/50/30/15 with content-driven computation
using QFontMetrics and tokens::spacing.

## Context
PlotScene::computePlotArea() uses fixed pixel margins to reserve
space for tick labels, axis labels, and the title:

```cpp
constexpr double kLeftMargin = 60.0;    // Y tick labels + Y axis label
constexpr double kBottomMargin = 50.0;  // X tick labels + X axis label
constexpr double kTopMarginWithTitle = 30.0;
constexpr double kTopMarginNoTitle = 15.0;
constexpr double kRightMargin = 15.0;
```

This violates the project rule "all visual values from style/
tokens" (CLAUDE.md rule 10, ADR-008). The correct approach is
to measure actual text extents at runtime using QFontMetrics and
derive margins from the longest tick label, the axis label height,
and the title height, plus DesignTokens::spacing padding.

## Decision
Accept the hardcoded margins for Phase 2. They produce acceptable
results for the reference data (electrophysiology CSV with
voltage/time ranges) and similar scientific datasets. Document
this as known tech debt with a Phase 4 refactor plan.

The hardcoded values were chosen empirically:
- Left 60px: accommodates 5-digit Y tick labels (e.g., "-38.50")
  in footnote font (12px) plus a rotated axis label.
- Bottom 50px: accommodates 4-digit X tick labels (e.g., "700")
  plus an axis label.
- Top 30px with title, 15px without: title in title-3 (17px) plus
  padding.
- Right 15px: minimal padding.

## Consequences
- + Fast implementation: no QFontMetrics dependency in PlotScene
- + Predictable layout: margins don't jump when data changes
- + Works well for the reference data and similar ranges
- - Breaks for very large numbers (e.g., 1,000,000 — tick labels
  clipped) or very long axis labels
- - Does not respect DesignTokens::spacing scale
- - Must be refactored before Phase 5 (property inspector), where
  users can set arbitrary axis labels

## Refactor plan (Phase 4)
Replace hardcoded constants with runtime computation:
1. Use QFontMetrics to measure the widest Y tick label
2. Add DesignTokens::spacing::md padding
3. Measure axis label height (rotated for Y)
4. Measure title height if present
5. Sum to get each margin
6. Cache and invalidate on font/data/label change

## Alternatives considered
- **Implement dynamic margins now**: rejected; Phase 2 goal is
  "first working plot", not pixel-perfect layout. Dynamic margins
  add complexity (font metrics, caching, invalidation) that is
  premature before the property inspector exists.
- **Use DesignTokens constants for margins**: better than raw
  numbers, but still wrong — the correct margin depends on text
  content, not a fixed token. Deferred to the full runtime
  computation in Phase 4.
