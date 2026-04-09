# ADR-020: InteractionController extracted from PlotCanvas

## Status
Accepted (Phase 3a); resolves ADR-016's deferred extraction.

## Context
ADR-016 documented that Phase 2's interaction logic (pan, zoom,
box-zoom, crosshair, reset) lives inline in PlotCanvas.cpp (~150
lines). The ADR specified extraction triggers: (a) adding a new
interaction mode, (b) PlotCanvas.cpp growing beyond 500 lines.

Phase 3a triggers condition (a): double-click on a line series
opens a property editor, while double-click on empty space resets
the view. This is a new interaction mode that requires HitTester
integration. Adding it inline to PlotCanvas would push the file
toward 300+ lines and mix hit-testing logic with rendering logic.

## Decision
Extract interaction logic into `InteractionController` in
`src/lumen/ui/`:

```cpp
namespace lumen::ui {

enum class InteractionMode { Idle, Panning, ZoomBoxing };

class InteractionController : public QObject {
    Q_OBJECT
public:
    explicit InteractionController(PlotCanvas* canvas);

    void handleMousePress(QMouseEvent* event);
    void handleMouseMove(QMouseEvent* event);
    void handleMouseRelease(QMouseEvent* event);
    void handleWheel(QWheelEvent* event);
    void handleDoubleClick(QMouseEvent* event);

    InteractionMode mode() const;
    QPointF lastMousePos() const;
    bool isZoomBoxActive() const;
    QRect zoomBoxRect() const;

signals:
    void seriesDoubleClicked(int seriesIndex);
    void emptyAreaDoubleClicked();
    void requestRepaint();
};
}
```

PlotCanvas becomes a thin rendering host:
- `paintEvent()` calls PlotRenderer, then draws overlays (zoom box,
  crosshair) using state from InteractionController.
- Mouse event overrides forward to `controller_->handleXxx()`.
- Connects controller signals to handle double-click outcomes.

InteractionController owns all interaction state (panning_, panStart_,
zoomBoxing_, zoomBoxStart_, lastMousePos_, mouseInWidget_) and all
interaction logic (pan computation, zoom computation, box-zoom
computation, hit-test dispatch).

### Double-click disambiguation
In `handleDoubleClick()`:
1. Compute CoordinateMapper from current scene view.
2. Call `HitTester::hitTest(scene, mapper, pixelPos)`.
3. If hit found: emit `seriesDoubleClicked(result.seriesIndex)`.
4. If no hit: emit `emptyAreaDoubleClicked()`.

PlotCanvasDock connects `seriesDoubleClicked` to open
LinePropertyDialog. PlotCanvas connects `emptyAreaDoubleClicked`
to `scene_->autoRange()`.

## Consequences
- + PlotCanvas is ~50 lines (paint + forward), easy to read
- + InteractionController is independently testable (unit test
  with mock PlotCanvas if needed, or test via HitTester directly)
- + New interaction modes (Phase 4: data selection, Phase 5:
  annotation placement) add methods to InteractionController
  without touching PlotCanvas
- + Clear signal-based communication: controller emits intent
  signals, PlotCanvasDock/PlotCanvas respond
- - Two classes instead of one; slightly more indirection
- - InteractionController needs a pointer to PlotCanvas (for
  scene access, size, cursor changes). Acceptable; it's an
  intimate collaborator, not a distant dependency.

## Alternatives considered
- **Keep interaction inline, add double-click logic**: rejected;
  ADR-016 trigger (a) is met. Inline approach would produce a
  300+ line PlotCanvas mixing rendering, hit-testing, and
  interaction state. Not maintainable as modes multiply.
- **State machine (QStateMachine)**: formal state machine for
  Idle→Panning→Idle transitions. Rejected; the transitions are
  simple (press→move→release) and don't benefit from QStateMachine's
  animation/transition infrastructure. The enum + if/else in
  InteractionController is clearer.
- **Strategy pattern with per-mode handler objects**: cleaner for
  many modes, but Phase 3a has only 4 modes (idle, pan, box-zoom,
  and the implicit crosshair-on-hover). Strategy adds 4 classes
  for ~30 lines of logic each. Premature. Revisit if modes exceed 7.
