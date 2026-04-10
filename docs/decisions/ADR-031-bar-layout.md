# ADR-031: Bar layout — relative width, median spacing, pixel minimum

## Status
Accepted (Phase 5)

## Context
BarSeries renders vertical bars from a Y=0 baseline to each data
point. The bar width must be computed from the data spacing (bars
should not overlap or leave large gaps) and must remain visible
at all zoom levels.

## Decision
Bar width = relativeWidth * medianXSpacing, with a minimum of 2
pixels.

- `relativeWidth`: user-configurable, range 0.1 to 1.0, default
  0.8. A value of 1.0 means bars touch; 0.8 leaves small gaps.
- `medianXSpacing`: the median of consecutive X-value differences,
  computed once when the series is created and cached. Median is
  more robust than mean against outliers (e.g., one large gap).
- `2px minimum`: when zoomed far out, bars would become sub-pixel.
  The minimum ensures they remain visible as thin lines.

Each bar occupies [x - w/2, x + w/2] horizontally and [0, y]
vertically (in data space), mapped to pixels via CoordinateMapper.

## Consequences
- + Bars adapt to data spacing: evenly-spaced data gets even bars,
  irregular spacing gets reasonable approximation via median
- + User-adjustable width for fine control
- + 2px minimum prevents disappearance at any zoom
- + Simple computation: one pass for median, one multiply per bar
- - Median spacing assumes roughly uniform data. Highly irregular
  X values produce inconsistent-looking bars. Acceptable for
  Phase 5; categorical X axis support deferred.
- - Single series per position: no grouped or stacked bars.
  Deferred to a later phase.

## Alternatives considered
- **Grouped bars**: multiple series share X positions, bars are
  side-by-side with offsets. Deferred: Phase 5 focuses on single-
  series bar plots. Grouped bars need a BarGroup concept.
- **Stacked bars**: bars stack vertically. Deferred: same reason.
- **Fixed pixel width**: bars are always N pixels wide regardless
  of data. Rejected: doesn't scale with zoom or data density.
  At high zoom, fixed-width bars overlap; at low zoom, they have
  excessive gaps.
