# ADR-024: HitTester precedence ordering for non-series elements

## Status
Accepted (Phase 3b)

## Context
Phase 3b extends HitTester to detect clicks on axes, title, and
legend in addition to line series (Phase 3a). Multiple hit regions
can overlap:

- The title rect may extend into the Y axis label band
- The legend frame may overlap with the plot area
- Axis tick label bands overlap with the plot area edges
- A line series passes through all of these regions

When a user double-clicks at a point that falls within multiple
regions, the system must choose one unambiguous target. The
precedence ordering determines which element "wins."

## Decision
Hit testing proceeds in this fixed precedence order (highest to
lowest):

1. **LineSeries** — checked first via hitTest() (Phase 3a). If a
   series segment is within 5px tolerance, it wins. This is
   highest because the user's most common edit action is changing
   line properties.

2. **Title** — if no series hit, check title rect. Title editing
   is common and the title area is small, so false positives are
   rare.

3. **Legend** — legend frame. Positioned inside or outside the
   plot area depending on Legend::position.

4. **XAxis / YAxis** — axis bands (tick labels, axis line, axis
   label). These occupy large screen regions, so they are low
   priority to avoid intercepting clicks meant for series.

5. **PlotArea** — fallback. Double-click on empty plot area resets
   to auto-range (Phase 2 behavior).

InteractionController implements this as:
```
result = hitTest(series)
if result: emit seriesDoubleClicked
else:
    region = hitNonSeriesElement()
    switch region.kind:
        Title:  emit titleDoubleClicked
        Legend: emit legendDoubleClicked
        XAxis:  emit xAxisDoubleClicked
        YAxis:  emit yAxisDoubleClicked
        _:      emit emptyAreaDoubleClicked
```

## Consequences
- + Deterministic: same click always targets the same element
- + Series get priority: most common interaction wins
- + Rare false positives: title and legend are small regions
- + Simple to implement: sequential checks, first match wins
- - Overlapping regions cannot both be targeted: if a legend
  overlaps a series segment, the series wins. Acceptable because
  the overlap area is tiny and the user can click slightly
  outside the series to hit the legend.
- - No visual disambiguation (e.g., context menu asking "did you
  mean: series or legend?"). Overkill for Phase 3b; revisit if
  users report confusion.

## Alternatives considered
- **Z-order based**: assign each element a z-index, the topmost
  element wins. Equivalent to our fixed precedence but adds
  unnecessary indirection (z-indices must be managed). Our
  elements have natural priority that doesn't change at runtime.
- **Smallest-rect-first**: the element with the smallest bounding
  rect at the click point wins (assumes smaller = more specific).
  Breaks for axis labels (small rects) that should have lower
  priority than series. Rejected.
- **Context menu disambiguation**: on ambiguous click, show a
  menu listing all hit elements. Adds a click to every edit
  action in overlap zones. Rejected for UX friction.
