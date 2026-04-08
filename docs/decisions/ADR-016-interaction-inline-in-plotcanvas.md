# ADR-016: Interaction logic is inline in PlotCanvas (not extracted)

## Status
Accepted (Phase 2); extraction deferred to Phase 4.

## Context
PlotCanvas handles five interaction modes: pan (left drag), zoom
(scroll wheel with modifier keys), zoom box (right drag), reset
(double-click), and crosshair (mouse hover). All interaction logic
lives directly in PlotCanvas's mouse event overrides
(mousePressEvent, mouseMoveEvent, mouseReleaseEvent, wheelEvent,
mouseDoubleClickEvent).

A more modular architecture would extract interaction into separate
classes: InteractionController (state machine managing modes),
PanHandler, ZoomHandler, ZoomBoxHandler, CrosshairOverlay. This
would make each handler independently testable and allow future
modes (e.g., data point selection, annotation placement) without
growing PlotCanvas.

## Decision
Keep interaction logic inline in PlotCanvas for Phase 2. The five
modes total ~150 lines of straightforward Qt mouse event handling.
Extracting them now would produce 5+ classes with 300+ lines of
boilerplate for the same functionality.

## Consequences
- + Simple: one file, one class, easy to read top-to-bottom
- + Fast to implement: no interface design needed
- + All interaction state (panning_, zoomBoxing_, lastMousePos_)
  is local to PlotCanvas
- - PlotCanvas grows with each new interaction mode; beyond 7-8
  modes it will become unwieldy
- - Interaction handlers are not independently unit-testable
  (cannot test pan logic without a PlotCanvas widget)
- - Adding interaction modes (Phase 4: data point selection;
  Phase 5: property inspector click) will require refactoring

## Extraction trigger (Phase 4)
Extract when any of:
- PlotCanvas exceeds 500 lines
- A third interaction mode is added beyond the initial five
- Data point hit-testing is needed (requires HitTester class)

## Alternatives considered
- **Extract InteractionController now**: rejected; premature
  abstraction for 5 simple handlers. The interface design depends
  on knowing what Phase 4-5 interactions look like, which we
  don't yet.
- **Qt state machine (QStateMachine)**: heavyweight for mouse
  interaction. State transitions are simple if/else on button +
  modifier state; no need for a formal state machine.
- **Strategy pattern per mode**: clean but adds vtable overhead
  and 5 classes for 150 lines of code. Net loss in simplicity.
