# ADR-039: Adaptive heatmap rendering — PlotCanvas hosts GPU layer

## Status
Accepted (Phase 7)

## Context
Heatmap rendering of large Grid2D datasets (>1M pixels) is too slow
for QPainter's CPU path at interactive framerates. A GPU path using
OpenGL texture rendering is 10-100× faster. The question is where
the GPU rendering infrastructure lives.

## Decision
PlotCanvas hosts a GPU layer via a lazily-created QOpenGLWidget
child positioned to overlay the plot area.

Architecture:
1. PlotCanvas::gpuLayer() creates a QOpenGLWidget child on first
   call (lazy — no GL overhead for CPU-only plots)
2. GPU widget is sized and positioned to match computePlotArea()
3. GPU widget has transparent background (Qt::WA_TranslucentBackground)
   so CPU-rendered elements (axes, labels, overlays) show through
4. Rendering order: CPU items via PlotRenderer → GPU widget renders
   GPU items (heatmap textures) → QPainter overlays (crosshair,
   zoom box) on top
5. If QOpenGLWidget creation fails (no GL driver), all items fall
   back to CPU path with a status bar warning

Threshold: grids ≤ 1024×1024 use CPU path, > 1024×1024 use GPU.
Rationale: CPU QPainter renders 1M pixels in ~4ms (60fps). Above
1M, frame time exceeds 16ms and interaction feels sluggish.

## Consequences
- + Generalizes to Phase 8 (3D volume rendering uses the same GPU
  widget with different shaders)
- + PlotCanvas manages one GPU widget for all GPU-capable items
- + Lazy creation: no GL overhead for line/scatter/bar plots
- + Transparent overlay: CPU and GPU content composited correctly
- - QOpenGLWidget adds ~20ms startup on first creation
- - Wayland compositor may handle transparency differently; test
  on both X11 and Wayland
- - GPU widget positioning must track plotArea on resize

## Alternatives considered
- **Always CPU (QPainter only)**: works for small grids but 4096×4096
  takes ~70ms per frame. Rejected for interactive use.
- **Always GPU (all rendering via OpenGL)**: maximum performance but
  adds GL driver dependency for simple line plots. Overkill and
  fragile. Rejected.
- **Heatmap owns its own QOpenGLWidget**: each Heatmap creates and
  manages its own GL widget. Rejected: multiple heatmaps would
  create multiple GL contexts (expensive), and the widget lifecycle
  is complex. PlotCanvas managing one shared GPU layer is simpler.
