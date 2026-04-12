# ADR-045: PlotItem3D as separate hierarchy from 2D PlotItem

## Status
Accepted (Phase 8)

## Context
Phase 5 established PlotItem as the abstract base for 2D plot
types. Phase 8 adds 3D plot types with fundamentally different
rendering requirements (OpenGL shaders vs QPainter, 3D transforms
vs 2D coordinate mapping, ray-cast hit-testing vs pixel-distance
hit-testing).

## Decision
PlotItem3D is a FULLY SEPARATE hierarchy from PlotItem. No shared
base class, no common "Renderable" interface.

**API differences that justify separation**:
- `PlotItem::paint(QPainter*, CoordinateMapper, QRectF)` vs
  `PlotItem3D::render(ShaderProgram&, RenderContext&)`
- `PlotItem::hitTestPoint(QPoint, CoordinateMapper, double)` vs
  `PlotItem3D::hitTestRay(Ray, double)`
- `PlotItem::dataBounds() → QRectF` vs
  `PlotItem3D::dataBounds() → BoundingBox3D`

**Cross-cutting concerns handled without shared base**:
- **CommandBus**: each command type knows its target (PlotItem* or
  PlotItem3D*). No unification needed.
- **WorkspaceFile**: `"plot"` block for 2D, `"plot3d"` block for
  3D. Discriminator-based dispatch on load.
- **HitTester**: PlotCanvas uses 2D hit-testing, PlotCanvas3D
  uses 3D ray-casting. Per-canvas, not per-item.
- **ReactiveBinding**: works identically for both — binds a
  Dataset to a plot item, mode-filtered. The binding doesn't
  care whether the target is 2D or 3D.

## Consequences
- + Clean separation: 2D and 3D evolve independently
- + No forced abstraction over fundamentally different APIs
- + Phase 9+ adds new types to either hierarchy without cross-
  contamination
- + Existing 2D code (Phases 1-7) completely untouched
- - Two type systems to manage in workspace serialization
  (manageable: two parallel blocks)
- - If future Phase needs mixed 2D+3D in one view, an adapter
  layer would be needed (not planned)

## Alternatives considered
- **Unified hierarchy** (common base "Renderable" with virtual
  paint/render): forces every PlotItem to handle a 3D context it
  doesn't need. Breaks Phase 5's clean design. Every existing
  test would need updating. Rejected.
- **Bridge pattern** (PlotItem holds a RenderStrategy): over-
  abstraction. The render strategies (QPainter vs GL) are so
  different that the bridge adds complexity without reducing it.
  Rejected.
