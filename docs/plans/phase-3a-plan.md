# Phase 3a Plan — Line Property Editing

> Reference: `docs/specs/phase-3a-spec.md`

## Task Dependency Graph

```
  ┌──────────────────────┐    ┌──────────────────────────┐
  │T1 CommandBus +       │    │T2 HitTester +            │
  │ Command base class   │    │ InteractionController    │
  │  (Backend)           │    │ extraction (Frontend)    │
  └──────────┬───────────┘    └──────────┬───────────────┘
             │                           │
             ▼                           │
  ┌──────────────────────┐               │
  │T3 LineSeries setters │               │
  │ + visibility         │               │
  │  (Backend)           │               │
  └──────────┬───────────┘               │
             │                           │
             └─────────┬─────────────────┘
                       ▼
            ┌──────────────────────┐
            │T4 LinePropertyDialog │
            │ + double-click flow  │
            │  (Frontend)          │
            └──────────┬───────────┘
                       │
            ┌──────────┼──────────────┐
            ▼          ▼              ▼
  ┌──────────────┐ ┌──────────┐ ┌──────────────┐
  │T5 Style      │ │T6 QA     │ │T7 Docs +     │
  │ persistence  │ │ tests    │ │ architecture │
  │ (Frontend)   │ │  (QA)    │ │ (Architect)  │
  └──────────────┘ └──────────┘ └──────────────┘
```

## Critical path

T1 + T2 (parallel) → T3 → T4 → T5

Estimated: ~8–12 working days. T6 and T7 run in parallel off
the critical path once T4 lands.

---

## Tasks

### T1 — CommandBus + Command Base Class (Backend, Size: L)

**Owner**: backend

**Files to create**:
- `src/lumen/core/Command.h` — abstract base class
- `src/lumen/core/CommandBus.h` / `.cpp` — executes, records, undoes
- `src/lumen/core/commands/ChangeLineStyleCommand.h` / `.cpp`
- `tests/unit/test_command_bus.cpp`

**Design** (see ADR-018):

```cpp
namespace lumen::core {

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    [[nodiscard]] virtual QString description() const = 0;
};

class CommandBus : public QObject {
    Q_OBJECT
public:
    void execute(std::unique_ptr<Command> cmd);
    void undo();
    void redo();
    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;
    [[nodiscard]] QString undoDescription() const;
    [[nodiscard]] QString redoDescription() const;
signals:
    void commandExecuted(const QString& description);
    void undoRedoStateChanged();
private:
    std::vector<std::unique_ptr<Command>> undoStack_;
    std::vector<std::unique_ptr<Command>> redoStack_;
};
}
```

`ChangeLineStyleCommand` is the first concrete command:
- Captures: PlotScene*, series index, old PlotStyle+name+visibility,
  new PlotStyle+name+visibility.
- `execute()` applies new style. `undo()` restores old style.

CommandBus owned by Application, passed through MainWindow.

**Subtasks**:
- T1.1: Command.h abstract base.
- T1.2: CommandBus.h/.cpp with execute/undo/redo, undo+redo stacks.
- T1.3: ChangeLineStyleCommand — first concrete command.
- T1.4: Wire CommandBus into Application, pass to MainWindow.
- T1.5: Unit tests (≥6): execute pushes to undo stack, undo
  reverses, redo re-applies, canUndo/canRedo state, undo clears
  redo stack on new execute, ChangeLineStyleCommand round-trip.

**Acceptance criteria**:
- CommandBus execute/undo/redo works
- ChangeLineStyleCommand modifies and restores LineSeries style
- 6+ tests pass under ASan+UBSan
- No changes to plot/ rendering code

**Dependencies**: none (can start immediately)

---

### T2 — HitTester + InteractionController Extraction (Frontend, Size: L)

**Owner**: frontend

**Files to create**:
- `src/lumen/plot/HitTester.h` / `.cpp`
- `src/lumen/ui/InteractionController.h` / `.cpp`
- `tests/unit/test_hit_tester.cpp`

**Files to modify**:
- `src/lumen/ui/PlotCanvas.h` / `.cpp` — thin host, delegates to
  InteractionController
- `src/lumen/plot/CMakeLists.txt` — add HitTester
- `src/lumen/CMakeLists.txt` — add InteractionController

**Subtasks**:

T2.1: `HitTester` (in plot/, UI-independent):
```cpp
namespace lumen::plot {
class HitTester {
public:
    struct HitResult {
        int seriesIndex = -1;
        double pixelDistance = 0.0;
    };

    static std::optional<HitResult> hitTest(
        const PlotScene& scene,
        const CoordinateMapper& mapper,
        QPointF pixelPos,
        double tolerancePx = 5.0);

private:
    static double pointToSegmentDistance(
        QPointF point, QPointF segA, QPointF segB);
};
}
```

Algorithm: for each visible series, build polylines, map each
segment to pixels, compute point-to-segment distance. Return
nearest series within tolerance. O(n) per series, acceptable for
Phase 3a data sizes.

Resolves ADR-017: HitTester is the foundation for nearest-point
crosshair upgrade (Phase 4). Phase 3a uses it for double-click
hit detection only.

T2.2: `InteractionController` (in ui/):
```cpp
namespace lumen::ui {

enum class InteractionMode {
    Idle,
    Panning,
    ZoomBoxing,
};

class InteractionController : public QObject {
    Q_OBJECT
public:
    explicit InteractionController(PlotCanvas* canvas);

    void handleMousePress(QMouseEvent* event);
    void handleMouseMove(QMouseEvent* event);
    void handleMouseRelease(QMouseEvent* event);
    void handleWheel(QWheelEvent* event);
    void handleDoubleClick(QMouseEvent* event);

    [[nodiscard]] InteractionMode mode() const;
    [[nodiscard]] QPointF lastMousePos() const;
    [[nodiscard]] bool isMouseInPlotArea() const;

    // Zoom box state (for PlotCanvas overlay drawing).
    [[nodiscard]] bool isZoomBoxActive() const;
    [[nodiscard]] QRect zoomBoxRect() const;

signals:
    void seriesDoubleClicked(int seriesIndex);
    void emptyAreaDoubleClicked();
    void requestRepaint();
};
}
```

Resolves ADR-016: interaction logic extracted from PlotCanvas into
a dedicated controller. PlotCanvas becomes a thin rendering host
that delegates all mouse events to InteractionController.

T2.3: Refactor PlotCanvas to use InteractionController:
- Remove all mouse state members (panning_, zoomBoxing_, etc.)
- Mouse event overrides forward to controller_->handleXxx()
- paintEvent still renders, but asks controller for overlay state
  (zoom box rect, crosshair position)
- Connect controller's signals to appropriate slots

T2.4: Unit tests for HitTester (≥5):
- Point exactly on a segment → correct series
- Point between two series → nearest series
- Point far from all series → nullopt
- Hidden series (visible=false) → skipped
- Empty scene → nullopt

T2.5: **Critical**: after refactoring PlotCanvas, manually verify
that all 7 Phase 2 interaction checks still work. Report in PR.

**Acceptance criteria**:
- HitTester finds correct series within tolerance
- InteractionController handles all 5 interaction modes
- PlotCanvas is a thin host (~50 lines of forwarding)
- All 7 Phase 2 interactions still work (human verification)
- 5+ HitTester tests pass under ASan+UBSan
- Existing 146 tests still pass

**Dependencies**: none (can start immediately, parallel with T1)

---

### T3 — LineSeries Mutability + Visibility (Backend, Size: S)

**Owner**: backend

**Files to modify**:
- `src/lumen/plot/LineSeries.h` / `.cpp` — add setters, visibility
- `src/lumen/plot/PlotScene.h` / `.cpp` — mutable series access
- `src/lumen/plot/PlotRenderer.cpp` — skip invisible series

**Subtasks**:
- T3.1: Add to LineSeries: `void setStyle(PlotStyle)`,
  `void setName(QString)`, `void setVisible(bool)`,
  `[[nodiscard]] bool isVisible() const`. Default visible=true.
- T3.2: PlotScene: `LineSeries& seriesAt(std::size_t index)` —
  mutable access for editing (existing `series()` returns const ref).
- T3.3: PlotRenderer: in the series drawing loop, skip series
  where `!series.isVisible()`. Legend also skips hidden series
  (or shows them greyed).
- T3.4: Unit tests: set style → verify changed, toggle visibility
  → verify isVisible(), invisible series excluded from rendering
  (test via PlotRenderer to QImage: render with visible=false,
  verify series color absent from image).

**Acceptance criteria**:
- LineSeries style/name/visibility are mutable via setters
- PlotRenderer skips invisible series
- dataRange() still includes invisible series (so zoom doesn't
  change when hiding a series)
- Tests pass under ASan+UBSan

**Dependencies**: T1 (ChangeLineStyleCommand needs setters to exist)

---

### T4 — LinePropertyDialog + Double-Click Flow (Frontend, Size: M)

**Owner**: frontend

**Files to create**:
- `src/lumen/ui/LinePropertyDialog.h` / `.cpp`

**Files to modify**:
- `src/lumen/ui/InteractionController.cpp` — emit seriesDoubleClicked
- `src/lumen/ui/PlotCanvasDock.cpp` — connect signal, open dialog,
  apply changes via CommandBus

**Subtasks**:

T4.1: `LinePropertyDialog` — QDialog with:
- Color button (QPushButton showing current color → QColorDialog)
- Line width spin box (QDoubleSpinBox, 0.5–10.0, step 0.5)
- Line style combo (Solid/Dash/Dot/DashDot/DashDotDot with
  preview delegates)
- Series name QLineEdit
- Visible QCheckBox
- OK / Cancel buttons (QDialogButtonBox)
- `void setStyle(const PlotStyle&, const QString& name, bool visible)`
- `PlotStyle resultStyle() const`
- `QString resultName() const`
- `bool resultVisible() const`

T4.2: Double-click flow in InteractionController:
- On double-click, call HitTester::hitTest().
- If hit found: emit `seriesDoubleClicked(hitResult.seriesIndex)`.
- If no hit: emit `emptyAreaDoubleClicked()`.

T4.3: PlotCanvasDock connects signals:
- `seriesDoubleClicked` → open LinePropertyDialog with series data,
  on OK create ChangeLineStyleCommand and execute via CommandBus.
- `emptyAreaDoubleClicked` → call scene_->autoRange(), repaint.

T4.4: Unit test for dialog data flow: construct dialog, set values
programmatically, verify result accessors return correct values.
(No GUI interaction — just data in/out.)

**Acceptance criteria**:
- Double-click on line opens dialog pre-filled with current style
- Change color/width/style/name/visible → OK → plot updates
- Double-click on empty space still resets view
- Changes go through CommandBus (undo works)
- Dialog test passes

**Dependencies**: T2 (HitTester + InteractionController), T3 (setters)

---

### T5 — Style Persistence Across Column Changes (Frontend, Size: S)

**Owner**: frontend

**Files to modify**:
- `src/lumen/ui/PlotCanvasDock.h` / `.cpp`

**Subtasks**:
- T5.1: Add `QHash<QString, PlotStyle> customStyles_` to
  PlotCanvasDock. Maps column name → user-customized style.
- T5.2: When LinePropertyDialog returns OK, store the custom style
  in `customStyles_[columnName]`.
- T5.3: In `rebuildPlot()`, after creating each LineSeries, check
  if `customStyles_` has an entry for that column name. If so,
  apply it via `series.setStyle(...)` and `series.setName(...)`.
- T5.4: Also store visibility in a parallel
  `QHash<QString, bool> customVisibility_`.

**Acceptance criteria**:
- Change line color → switch Y column → switch back → color preserved
- Custom name preserved across column switches
- Visibility state preserved

**Dependencies**: T4 (dialog creates the custom styles)

---

### T6 — QA Tests (QA, Size: M)

**Owner**: qa

**Files to create/modify**:
- `tests/unit/test_hit_tester.cpp` (extend if T2 created it)
- `tests/unit/test_line_property_dialog.cpp`
- `tests/unit/test_command_bus.cpp` (extend if T1 created it)
- `tests/integration/test_line_editing.cpp`

**Subtasks**:
- T6.1: Review and extend HitTester tests: edge cases (overlapping
  series, NaN gaps near hit point, zoomed-in view).
- T6.2: Review and extend CommandBus tests: multiple undo/redo
  cycles, undo after redo, max stack depth.
- T6.3: Integration test: load CSV → auto-plot → change series
  style via code (simulate dialog) → verify PlotScene reflects
  change → undo → verify restored.
- T6.4: Integration test: verify invisible series excluded from
  rendered image but included in auto-range.

**Acceptance criteria**:
- Total test count ≥ 165
- All pass under ASan+UBSan

**Dependencies**: T4 (tests exercise the full editing flow)

---

### T7 — Architecture Update + STATUS (Architect, Size: S)

**Owner**: architect

**Files to modify**:
- `docs/architecture.md` — add Phase 3a section
- `.lumen-ops/STATUS.md` — Phase 3a closing entry

**Content**:
- Architecture: CommandBus data flow, HitTester in plot/ layer,
  InteractionController in ui/ layer, PlotCanvas as thin host,
  edit flow (double-click → HitTester → dialog → Command → execute)
- STATUS: tasks delivered, test count, human verification result

**Dependencies**: T5, T6 (after all features merged)

---

## Parallel Execution Schedule

```
Round 1 (parallel, ~3h):
  Backend:   T1 (CommandBus + Command base + ChangeLineStyleCommand)
  Frontend:  T2 (HitTester + InteractionController extraction)

Round 2 (~2h):
  Backend:   T3 (LineSeries setters + visibility)

Round 3 (~3h):
  Frontend:  T4 (LinePropertyDialog + double-click flow)

Round 4 (parallel, ~2h):
  Frontend:  T5 (style persistence)
  QA:        T6 (tests)

Round 5 (~30min):
  Architect: T7 (architecture + STATUS)

Round 6:
  Human verification (9 acceptance checks from spec)
```

**Total wall time**: ~10–12 hours across rounds.

## Risks

- **T2 PlotCanvas refactor may break interactions**: the extraction
  of ~150 lines of inline mouse handling into InteractionController
  is the riskiest change. Mitigated by: Frontend must verify all 7
  Phase 2 interactions before opening PR.
- **CommandBus adds boilerplate to every edit**: acceptable; it's a
  foundation investment. Phase 3a has only one command type
  (ChangeLineStyleCommand). Phase 4+ will add more.
- **HitTester performance on many segments**: O(n) per series.
  For 3,500 × 2 = 7,000 segments, ~0.5ms. Acceptable.

## Phase 2.5 Lessons Applied

- Coordinator implements directly when subagents are blocked by
  sandbox permissions. Budget time for this.
- QA exclusively owns test files.
- Build verification with `QT_QPA_PLATFORM=offscreen` and
  `ASAN_OPTIONS=detect_leaks=0` before every commit.
