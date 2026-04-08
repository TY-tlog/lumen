# ADR-003: The plot engine is built in-house on QPainter

## Status
Accepted (Phase 0)

## Context
The owner explicitly wants a MATLAB-figure-class interactive plot
engine, with full control over the object model (so that
double-clicking any element opens a property inspector reflecting
that element's properties). Existing libraries (Qt Charts,
QCustomPlot, Qwt) are either too limited or impose object models
that conflict with this goal. The owner also values writing the
core code by hand.

## Decision
Implement the plot engine ourselves on top of QPainter (and
optionally QOpenGLWidget for high-density data later). No external
plot library. Phase 2 covers the basic engine; Phase 5 adds the
property inspector.

## Consequences
- + Complete control over object model, hit-testing, rendering
- + No third-party API churn
- + The engine is small and tailored exactly to our needs
- - Significant work (estimated 4–6 months total across phases)
- - We must rediscover edge cases (HiDPI, font metrics, dashed
  lines, anti-aliasing) that mature libraries already solved
- - Performance optimization (LOD, decimation) is on us

## Alternatives considered
- QCustomPlot: object model too limited for property-inspector flow
- Qt Charts: limited interaction, restrictive API
- Qwt: dated API
