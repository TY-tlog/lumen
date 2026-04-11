# ADR-041: CONREC-grade contour algorithm, from scratch

## Status
Accepted (Phase 7)

## Context
Contour line extraction from a 2D scalar field is needed for
ContourPlot. The algorithm must handle degenerate cases (saddle
points, plateaus, NaN cells) and produce topologically consistent
contour lines.

## Decision
Implement a CONREC-grade algorithm from scratch in Lumen's
codebase, based on Paul Bourke's description (Byte magazine, 1987;
also available at paulbourke.net/papers/conrec/).

The algorithm:
1. For each grid cell (2×2 corners), subdivide into 4 triangles
   using the cell center value (average of corners).
2. For each triangle and each contour level, determine if the
   level intersects the triangle edges.
3. If yes, interpolate the intersection points and emit a
   ContourSegment (line segment at that level).
4. Accumulate segments into contiguous polylines for rendering.

Degenerate handling:
- **Saddle point** (diagonal corners above and below level):
  triangular subdivision resolves the ambiguity deterministically
  (the center value picks one topology).
- **Plateau** (value exactly on level at a corner): perturb by
  epsilon (1e-10) to avoid zero-length segments.
- **NaN cell**: skip the cell entirely; contours stop at NaN
  boundaries.

## Consequences
- + Full control over output format (ContourSegment is Lumen's
  own type, not a library's)
- + Deterministic saddle point handling via triangular subdivision
- + No external dependency
- + Contour segments directly compatible with QPainter::drawLine
- - Implementation effort: ~200-300 lines
- - Must test degenerate cases thoroughly (saddle, plateau, NaN,
  single-cell grid, uniform grid)
- - No automatic contour line smoothing (segments are piecewise
  linear); Phase 9 can add Bezier smoothing

## Alternatives considered
- **Basic marching squares** (without triangular subdivision):
  simpler but ambiguous at saddle points. The ambiguity produces
  inconsistent contour topology. Rejected.
- **External library** (contourpy, VTK marching_cubes): adds a
  heavyweight dependency. contourpy is Python-only. VTK is massive.
  Rejected for the same reason as other external libraries
  (CLAUDE.md rule: self-built core).
- **Sutherland-Hodgman clipping approach**: produces filled contour
  bands, not contour lines. Different output format. Rejected for
  Phase 7 (lines first); filled contours can be a Phase 9 addition.
