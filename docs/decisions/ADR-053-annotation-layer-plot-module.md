# ADR-053: Annotation layer in plot/ module

## Status
Proposed (Phase 9)

## Context
Phase 9.5 adds annotations (arrows, boxes, callouts, text,
scale bar, color bar) to plots. The layer must support three
anchor modes (Data, Pixel, AxisFraction), hit-testing for
double-click editing, and workspace serialization.

## Decision
AnnotationLayer lives in src/lumen/plot/, owned by PlotScene
(not PlotCanvas).

### Ownership model
```
PlotScene
  ├── items_: vector<unique_ptr<PlotItem>>     (2D series)
  ├── annotationLayer_: AnnotationLayer         (NEW)
  │     └── annotations_: vector<unique_ptr<Annotation>>
  ├── xAxis_, yAxis_
  ├── title_, legend_
  └── ...
```

PlotRenderer paints annotations AFTER PlotItems (annotations
overlay data). AnnotationLayer::paint() receives the same
QPainter and CoordinateMapper as PlotItems.

### Coordinate model
```
Annotation::Anchor::Data        → CoordinateMapper transforms
Annotation::Anchor::Pixel       → absolute pixel position
Annotation::Anchor::AxisFraction → fractional (0..1) of plot area
```

Data-anchored annotations move with pan/zoom automatically
(CoordinateMapper recalculates on every paint). No reactive
binding needed — annotations don't observe Dataset changes;
they observe viewport changes via the existing repaint cycle.

### 3D annotations: deferred
Annotations are 2D-only. On PlotCanvas3D, they render as a
screen-space QPainter overlay on top of the 3D viewport (same
technique as the 2D crosshair overlay). 3D-space annotations
(labels floating in 3D coordinate space) are explicitly out of
scope for Phase 9.

### Hit-testing
AnnotationLayer::hitTest(QPoint pixel) iterates annotations in
reverse paint order (top-most first), checks boundingRect()
containment. Returns annotation ID or nullopt.

InteractionController priority: annotation hit > series hit >
non-series element hit.

### Workspace serialization
WorkspaceFile gains "annotations" array in the "plot" block:
```json
{
  "plot": {
    "annotations": [
      { "type": "arrow", "anchor": "data", ... },
      { "type": "text", "anchor": "axis_fraction", ... }
    ]
  }
}
```

Each Annotation subclass implements toJson()/fromJson().

## Consequences
- + Annotations live with the data they describe (PlotScene)
- + Layering preserved: plot/ doesn't import from ui/
- + WorkspaceFile serialization natural (PlotScene → JSON)
- + ADR-026 single-code-path preserved (annotations rendered
  via same QPainter)
- + Phase 11 Dashboard inherits: each PlotScene carries its
  own AnnotationLayer
- - PlotScene grows in complexity (~50 LOC for AnnotationLayer
  integration)
- - 6 annotation types is significant implementation scope

## Alternatives considered
- **Separate annotation/ module**: Rejected. Annotations share
  the CoordinateMapper and paint alongside PlotItems. A separate
  module would need to import from plot/ and create a circular
  concern — plot rendering needs to know about annotations for
  paint ordering.
- **Annotations on PlotCanvas** (UI layer): Rejected. Violates
  layering (UI concept leaking into scene model). WorkspaceFile
  (core/io/) can't access UI layer. Serialization broken.
- **Per-PlotItem annotations** (each series owns its annotations):
  Rejected. Annotations span the plot area, not bound to one
  series. A callout might point to a region, not a specific
  series. Scale bars and color bars are plot-global.
