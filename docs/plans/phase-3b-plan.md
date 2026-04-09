# Phase 3b Plan — Axis, Title, Legend Editing + Dynamic Margins

> Reference: `docs/specs/phase-3b-spec.md`

## Preamble: pattern deviations from spec

Phase 3b follows three implementation patterns established in
Phase 3a that deviate from the original spec. All agents must
follow these patterns, not the spec's original patterns.

### 1. Bundled command pattern (not individual SetXxx commands)

Phase 3a's ChangeLineStyleCommand captures style+name+visibility
as one undo unit per dialog confirmation. Phase 3b follows suit:

- ChangeAxisPropertiesCommand (label + range mode + manual min/max
  + tick count + tick format + grid visible)
- ChangeTitleCommand (text + font size + weight)
- ChangeLegendCommand (position + visible)

This matches dialog UX: user edits multiple properties → OK →
one undo entry. The spec's individual SetAxisLabelCommand,
SetAxisRangeCommand, etc. are NOT implemented.

### 2. Separate signal pattern (not generic editRequested)

Phase 3a's InteractionController emits typed signals
(seriesDoubleClicked, emptyAreaDoubleClicked). Phase 3b adds:

- xAxisDoubleClicked()
- yAxisDoubleClicked()
- titleDoubleClicked()
- legendDoubleClicked()

Each signal is strongly typed. PlotCanvasDock connects each to
the appropriate dialog open call. The spec's generic
editRequested(HitResult) is NOT implemented.

### 3. HitTester extension via new methods (not interface refactor)

Phase 3a's HitTester::hitTest() returns series hits only. Phase
3b adds two new methods without modifying hitTest():

- hitNonSeriesElement(scene, mapper, pixelPos) → HitKind enum
  (XAxis, YAxis, Title, Legend, PlotArea, None)
- hitTestPoint(scene, mapper, pixelPos, maxPixelDist) →
  optional<PointHitResult{seriesIndex, sampleIndex, dataPoint,
  pixelDistance}>

### ADR resolutions in this phase

- **ADR-013** (hardcoded margins) → resolved by T6
  (PlotScene::computeMargins)
- **ADR-017** (crosshair cursor) → resolved by T6.5
  (hitTestPoint + crosshair upgrade)

---

## Task Dependency Graph

```
  ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐
  │T1 Axis │ │T2 Title│ │T3 Leg- │ │T5 Hit- │ │T6 com- │
  │setters │ │state   │ │end cls │ │Tester  │ │pute-   │
  │(back)  │ │(back)  │ │(back)  │ │extend  │ │Margins │
  └───┬────┘ └───┬────┘ └───┬────┘ │(front) │ │(front) │
      │          │          │      └───┬────┘ └───┬────┘
      └────┬─────┘──────────┘          │          │
           ▼                           │          │
     ┌───────────┐                     │          │
     │T4 Bundled │                     │          │
     │ commands  │                     │          │
     │ (backend) │                     │          │
     └─────┬─────┘                     │          │
           │          ┌────────────────┘          │
           │          │                           │
           ▼          ▼                           │
     ┌──────────┐ ┌──────────┐ ┌──────────┐      │
     │T7 Axis-  │ │T8 Title- │ │T9 Legend-│      │
     │ Dialog   │ │ Dialog + │ │ Dialog   │      │
     │(front)   │ │ inline   │ │(front)   │      │
     │          │ │(front)   │ │          │      │
     └────┬─────┘ └────┬─────┘ └────┬─────┘      │
          │            │            │             │
          └─────┬──────┘────────────┘             │
                ▼                                 │
          ┌───────────┐                           │
          │T10 Canvas │                           │
          │ dispatch  │←──────────────────────────┘
          │(front)    │
          └─────┬─────┘
                │
                ▼
          ┌───────────┐
          │T6.5 hit-  │
          │TestPoint +│
          │ crosshair │
          │(front)    │
          └─────┬─────┘
                │
       ┌────────┼────────────┐
       ▼        ▼            ▼
  ┌────────┐ ┌────────┐ ┌────────┐
  │T11 QA  │ │T12 QA  │ │T13 QA  │
  │ unit   │ │ integ  │ │ regres │
  └────────┘ └────────┘ └────────┘
       │        │            │
       └────────┼────────────┘
                ▼
          ┌───────────┐
          │T14 Integ  │
          │T15 Docs   │
          └───────────┘
```

## Critical path

T1+T2+T3+T5+T6 (parallel) → T4 → T7+T8+T9 (parallel) → T10
→ T6.5 → T13 → T14

Estimated: ~14-18 working days. Many tasks run in parallel.

---

## Tasks

### T1 — Axis Setters + Changed Signal (Backend, Size: M)

**Owner**: backend

**Files to modify**:
- `src/lumen/plot/Axis.h` / `.cpp`

**Files to create**:
- `tests/unit/test_axis_setters.cpp`

**What to add to Axis**:

```cpp
// New enums
enum class RangeMode { Auto, Manual };
enum class TickFormat { Auto, Scientific, Fixed };

// New setters (each compares old vs new, emits changed())
void setLabel(const QString& label);
void setRangeMode(RangeMode mode);
void setManualRange(double min, double max);
void setTickCount(int count);  // 0 = auto
void setTickFormat(TickFormat format);
void setTickFormatDecimals(int n);  // for Fixed mode
void setGridVisible(bool visible);

// New getters
[[nodiscard]] RangeMode rangeMode() const;
[[nodiscard]] int tickCount() const;
[[nodiscard]] TickFormat tickFormat() const;
[[nodiscard]] int tickFormatDecimals() const;
[[nodiscard]] bool gridVisible() const;

// Signal
signals:
    void changed();
```

Axis must become a QObject to emit signals. Currently it is NOT
a QObject. **This is a significant change** — Axis can no longer
be stored in std::vector by value (QObjects are not copyable).
PlotScene must store axes as members (it already does: xAxis_,
yAxis_), not in a vector. This should work without issue.

**Acceptance criteria**:
- All setters store values and emit changed() when value differs
- ticks() respects tickCount and tickFormat
- Manual range mode overrides autoRange
- 6+ unit tests pass under ASan+UBSan

**Dependencies**: none (day 1)

---

### T2 — PlotScene Title State Expansion (Backend, Size: S)

**Owner**: backend

**Files to modify**:
- `src/lumen/plot/PlotScene.h` / `.cpp`

**Files to create**:
- `tests/unit/test_plot_scene_title.cpp`

**What to add**:
- `void setTitleFontPx(int px)` / `int titleFontPx() const`
  (default: tokens::typography::title3.sizePx = 17)
- `void setTitleWeight(QFont::Weight w)` / `QFont::Weight titleWeight() const`
  (default: QFont::DemiBold)
- Emit a signal or notify when title state changes

**Acceptance criteria**:
- Title font size and weight are stored and retrievable
- PlotRenderer uses these values when drawing the title
- 3+ unit tests

**Dependencies**: none (day 1)

---

### T3 — Legend Class Extraction (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/Legend.h` / `.cpp`
- `tests/unit/test_legend.cpp`

**Files to modify**:
- `src/lumen/plot/PlotScene.h` / `.cpp` — own a Legend instance
- `src/lumen/plot/PlotRenderer.cpp` — read Legend state from
  PlotScene instead of hardcoding legend rendering
- `src/lumen/plot/CMakeLists.txt` — add Legend sources

**Legend class**:
```cpp
namespace lumen::plot {

enum class LegendPosition {
    TopLeft, TopRight, BottomLeft, BottomRight, OutsideRight
};

class Legend : public QObject {
    Q_OBJECT
public:
    explicit Legend(QObject* parent = nullptr);

    void setPosition(LegendPosition pos);
    [[nodiscard]] LegendPosition position() const;

    void setVisible(bool visible);
    [[nodiscard]] bool isVisible() const;

signals:
    void changed();

private:
    LegendPosition position_ = LegendPosition::TopRight;
    bool visible_ = true;
};
}
```

Legend does NOT own series data — it reads entries from
PlotScene::series() at render time (name + color from each
LineSeries).

PlotRenderer's existing legend drawing code (~30 lines) moves to
use Legend::position() for placement and Legend::isVisible() for
visibility, instead of hardcoding TopRight and always-visible.

**Acceptance criteria**:
- Legend positions correctly at all 5 positions
- OutsideRight position influences PlotScene margins (T6 uses
  this)
- Invisible legend is not rendered
- 4+ unit tests

**Dependencies**: none (day 1)

---

### T4 — Bundled Commands (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/core/commands/ChangeAxisPropertiesCommand.h` / `.cpp`
- `src/lumen/core/commands/ChangeTitleCommand.h` / `.cpp`
- `src/lumen/core/commands/ChangeLegendCommand.h` / `.cpp`
- `tests/unit/test_axis_commands.cpp`
- `tests/unit/test_title_command.cpp`
- `tests/unit/test_legend_commands.cpp`

**ChangeAxisPropertiesCommand**:
Captures: Axis* pointer, old/new values for all 7 properties
(label, rangeMode, manualMin, manualMax, tickCount, tickFormat,
tickFormatDecimals, gridVisible). Execute applies new, undo
restores old.

**ChangeTitleCommand**:
Captures: PlotScene* pointer, old/new title text, font size,
weight. Execute applies new, undo restores old.

**ChangeLegendCommand**:
Captures: Legend* pointer, old/new position, visible. Execute
applies new, undo restores old.

**SetSeriesNameCommand** (thin):
If ChangeLineStyleCommand from Phase 3a cannot be reused for
legend-only name edits (it captures the full style), add a thin
SetSeriesNameCommand that captures PlotScene*, seriesIndex,
old/new name only. Or reuse ChangeLineStyleCommand with same
style but different name — implementer's choice.

**Acceptance criteria**:
- Each command's execute/undo correctly modifies and restores
- Commands work through CommandBus (undo/redo)
- 5+ tests for AxisCommand, 2+ for TitleCommand, 3+ for
  LegendCommand

**Dependencies**: T1, T2, T3 (setters must exist)

---

### T5 — HitTester hitNonSeriesElement Extension (Frontend, Size: M)

**Owner**: frontend

**Files to modify**:
- `src/lumen/plot/HitTester.h` / `.cpp`

**Files to create**:
- `tests/unit/test_hit_tester_regions.cpp`

**New method and enum**:
```cpp
enum class HitKind {
    None, XAxis, YAxis, Title, Legend, PlotArea
};

struct RegionHitResult {
    HitKind kind = HitKind::None;
};

static RegionHitResult hitNonSeriesElement(
    const PlotScene& scene,
    const CoordinateMapper& mapper,
    QPointF pixelPos);
```

Algorithm: given pixel position, check (in precedence order per
ADR-024):
1. Title rect (from PlotScene layout) → HitKind::Title
2. Legend rect (from PlotScene/Legend layout) → HitKind::Legend
3. X axis band (bottom margin area) → HitKind::XAxis
4. Y axis band (left margin area) → HitKind::YAxis
5. Inside plot area → HitKind::PlotArea
6. Otherwise → HitKind::None

Note: LineSeries hit testing is NOT in this method — it stays in
the existing hitTest(). InteractionController calls hitTest()
first; if no series hit, calls hitNonSeriesElement().

Rect computations come from PlotScene::computePlotArea() and the
margin values.

**Acceptance criteria**:
- Correct hit kind for each region
- Precedence: Title > Legend > XAxis/YAxis > PlotArea
- 6+ tests

**Dependencies**: none (day 1)

---

### T6 — PlotScene::computeMargins (Frontend, Size: M) — RESOLVES ADR-013

**Owner**: frontend

**Files to modify**:
- `src/lumen/plot/PlotScene.h` / `.cpp`

**Files to create**:
- `tests/unit/test_plot_scene_margins.cpp`

**Replace hardcoded margins with dynamic computation**:

```cpp
struct PlotMargins {
    double left = 0.0;
    double top = 0.0;
    double right = 0.0;
    double bottom = 0.0;
};

PlotMargins computeMargins(const QFontMetrics& tickFm,
                           const QFontMetrics& labelFm,
                           const QFontMetrics& titleFm) const;
```

Implementation:
- left = max Y tick label width (via tickFm.horizontalAdvance on
  formatted tick strings) + tokens::spacing::md + Y axis label
  height (via labelFm.height()) + tokens::spacing::sm
- bottom = X tick label height (tickFm.height()) +
  tokens::spacing::md + X axis label height (labelFm.height())
  + tokens::spacing::sm
- top = title present ? titleFm.height() + tokens::spacing::md
  : tokens::spacing::sm
- right = legend position == OutsideRight ? legend width +
  tokens::spacing::md : tokens::spacing::md

computePlotArea() calls computeMargins() instead of using
hardcoded 60/50/30/15.

**Debounce**: Cache last computed margins. If new margins differ
by ≤1 pixel in all dimensions, reuse cached values to prevent
jiggle during live edits.

**Acceptance criteria**:
- Small font → smaller margins than hardcoded 60/50/30/15
- Long Y label → wider left margin
- Title presence → taller top margin
- OutsideRight legend → wider right margin
- 1-pixel debounce threshold prevents jiggle
- 5+ tests
- **This resolves ADR-013.**

**Dependencies**: none (day 1, but T3 Legend must land before
OutsideRight margin logic is testable)

---

### T6.5 — hitTestPoint + Crosshair Upgrade (Frontend, Size: M) — RESOLVES ADR-017

**Owner**: frontend

**Files to modify**:
- `src/lumen/plot/HitTester.h` / `.cpp`
- `src/lumen/ui/PlotCanvas.cpp` (crosshair rendering)

**Files to create**:
- `tests/unit/test_hit_test_point.cpp`

**New method**:
```cpp
struct PointHitResult {
    int seriesIndex = -1;
    std::size_t sampleIndex = 0;
    QPointF dataPoint;
    double pixelDistance = 0.0;
};

static std::optional<PointHitResult> hitTestPoint(
    const PlotScene& scene,
    const CoordinateMapper& mapper,
    QPointF pixelPos,
    double maxPixelDistance = 20.0);
```

Algorithm: for each visible LineSeries:
1. Get X column doubleData() (assumed monotonic for
   electrophysiology time series).
2. Map pixelPos.x() to dataX via mapper.pixelToData().
3. Binary search X data for the two samples bracketing dataX.
4. Map those samples to pixel space.
5. Compute Euclidean pixel distance from pixelPos to each.
6. Track the global minimum across all series.
7. If minimum ≤ maxPixelDistance, return PointHitResult.

**Crosshair upgrade in PlotCanvas**:
- In drawCrosshair(), call hitTestPoint() instead of
  CoordinateMapper::pixelToData().
- If hit found: draw crosshair at the snapped sample pixel
  position, show tooltip "(x_value, y_value)" with actual data
  values.
- If no hit: hide crosshair entirely (no interpolated display).
- Status bar: "Series: name, Sample N: (x, y)" when snapped.

**Acceptance criteria**:
- Hovering near spike peak shows actual peak voltage, not
  interpolated value
- Crosshair snaps visually to data points
- Hidden when cursor far from data
- Works correctly with NaN gaps
- 5+ tests
- **This resolves ADR-017.**

**Dependencies**: T5 (uses HitTester pattern)

---

### T7 — AxisDialog (Frontend, Size: M)

**Owner**: frontend

**Files to create**:
- `src/lumen/ui/AxisDialog.h` / `.cpp`

**Non-modal QDialog** following LinePropertyDialog pattern:
- QLineEdit for label
- QButtonGroup (Auto/Manual) for range mode
- Two QDoubleSpinBox for min/max (visible only in Manual mode)
- QSpinBox for tick count (0 = Auto)
- QComboBox for tick format (Auto/Scientific/Fixed 0-6)
- QCheckBox for grid visibility
- OK/Cancel via QDialogButtonBox

On OK: create ChangeAxisPropertiesCommand, execute via CommandBus.
On Cancel: revert (or simply don't create a command).

**Acceptance criteria**:
- Dialog opens pre-filled with current axis properties
- Changes apply on OK, visible immediately
- Cancel leaves axis unchanged
- Undo restores previous state

**Dependencies**: T4 (command), T5 (HitTester for double-click dispatch)

---

### T8 — TitleDialog + Inline Title Editor (Frontend, Size: M)

**Owner**: frontend

**Files to create**:
- `src/lumen/ui/TitleDialog.h` / `.cpp`

**Files to modify**:
- `src/lumen/ui/PlotCanvas.h` / `.cpp` (inline editor overlay)
- `src/lumen/ui/InteractionController.h` / `.cpp`
  (EditingTitleInline mode)

**TitleDialog** (right-click only): font size QSpinBox, weight
QComboBox (Normal/Medium/Bold). On OK: ChangeTitleCommand.

**Inline editor** (double-click on title area):
- QLineEdit positioned absolutely at the title rect
- Same font as title (titleFontPx, titleWeight)
- Pre-filled with current title text (empty if no title)
- Enter → ChangeTitleCommand, hide editor
- Esc → hide editor, no command
- Focus lost → apply (same as Enter)

InteractionController gets a new mode EditingTitleInline. While
in this mode, mouse events are suppressed (no pan/zoom).

**Acceptance criteria**:
- Double-click title area → inline editor appears
- Typing + Enter → title updates, margin adjusts
- Esc → no change
- Right-click title → TitleDialog opens
- Undo restores previous title

**Dependencies**: T4 (command), T5 (HitTester region detection)

---

### T9 — LegendDialog (Frontend, Size: M)

**Owner**: frontend

**Files to create**:
- `src/lumen/ui/LegendDialog.h` / `.cpp`

**Non-modal QDialog**:
- QComboBox for position (5 options)
- QCheckBox for visibility
- QTableWidget: two columns (Series name editable, Color swatch
  read-only)

On OK: ChangeLegendCommand for position/visibility changes.
Series name edits in the table: create ChangeLineStyleCommand
(reuse Phase 3a command with same style but new name) per edited
row.

**Acceptance criteria**:
- Dialog opens with current legend state
- Position change → legend moves immediately
- OutsideRight → right margin grows (via T6 computeMargins)
- Series rename → name updates in plot legend and column picker
- Undo works

**Dependencies**: T4 (command), T5 (HitTester region detection)

---

### T10 — PlotCanvas Dispatch to Four Dialogs (Frontend, Size: M)

**Owner**: frontend

**Files to modify**:
- `src/lumen/ui/InteractionController.h` / `.cpp`
- `src/lumen/ui/PlotCanvasDock.h` / `.cpp`
- `src/lumen/ui/PlotCanvas.h` / `.cpp`

**InteractionController additions**:
- In handleDoubleClick: after checking hitTest() for series (Phase
  3a, unchanged), call hitNonSeriesElement(). Based on HitKind:
  - XAxis → emit xAxisDoubleClicked()
  - YAxis → emit yAxisDoubleClicked()
  - Title → emit titleDoubleClicked()
  - Legend → emit legendDoubleClicked()
  - PlotArea (no series, no element) → emit
    emptyAreaDoubleClicked() (Phase 3a, unchanged)

**New signals**:
```cpp
signals:
    // Phase 3a (unchanged)
    void seriesDoubleClicked(int seriesIndex);
    void emptyAreaDoubleClicked();
    void requestRepaint();

    // Phase 3b
    void xAxisDoubleClicked();
    void yAxisDoubleClicked();
    void titleDoubleClicked();
    void legendDoubleClicked();
```

**PlotCanvasDock connects new signals**:
- xAxisDoubleClicked → open AxisDialog for X axis
- yAxisDoubleClicked → open AxisDialog for Y axis
- titleDoubleClicked → start inline title editor
- legendDoubleClicked → open LegendDialog

**PlotCanvas additions**:
- Hover highlighting: in paintEvent, after PlotRenderer, if mouse
  is over an axis/title/legend region (query hitNonSeriesElement),
  draw a subtle highlight (1px accent border or slight brightness).
- Manage inline title editor lifecycle (create/show/hide QLineEdit).

**Acceptance criteria**:
- Double-click each element type opens the correct dialog
- Phase 3a line editing still works (seriesDoubleClicked unchanged)
- Hover highlighting visible on axes, title, legend
- All 7 Phase 2 interactions still work

**Dependencies**: T7, T8, T9 (dialogs must exist to connect)

---

### T11 — QA Unit Tests (QA, Size: L)

**Owner**: qa

**Files to create**:
- `tests/unit/test_axis_setters.cpp` (6+ tests)
- `tests/unit/test_plot_scene_title.cpp` (3+ tests)
- `tests/unit/test_legend.cpp` (4+ tests)
- `tests/unit/test_axis_commands.cpp` (5+ tests)
- `tests/unit/test_title_command.cpp` (2+ tests)
- `tests/unit/test_legend_commands.cpp` (3+ tests)
- `tests/unit/test_hit_tester_regions.cpp` (6+ tests)
- `tests/unit/test_plot_scene_margins.cpp` (5+ tests)
- `tests/unit/test_hit_test_point.cpp` (5+ tests)

**Target**: 39+ new unit tests, total ≥ 200.

**Dependencies**: tests written as modules land

---

### T12 — QA Integration Tests (QA, Size: M)

**Owner**: qa

**Files to create**:
- `tests/integration/test_axis_dialog_edit.cpp`
- `tests/integration/test_title_inline_edit.cpp`
- `tests/integration/test_legend_dialog_edit.cpp`
- `tests/integration/test_dynamic_margin_no_clip.cpp`
- `tests/integration/test_crosshair_nearest_point.cpp`

**Dependencies**: T10 (full dispatch wired), T6.5 (crosshair)

---

### T13 — Phase 3a Regression Check (QA, Size: S) — MANDATORY

**Owner**: qa

**File to create**:
- `tests/integration/test_phase_3a_regression.cpp`

Tests:
- Double-click on a line series → HitTester returns series index
  (not axis/title/legend)
- LinePropertyDialog can be constructed and returns valid data
- Phase 2 interactions: pan, wheel zoom, box zoom verified
  programmatically

**This test must pass before any Frontend merge in Phase 3b.**

**Dependencies**: T10 (after PlotCanvas changes)

---

### T14 — Integration: Merges + Tag (Integration, Size: S)

**Owner**: integration

Merge PRs as they land. Tag vphase-3b after all tests pass and
human verifies.

---

### T15 — Docs Update (Docs, Size: S)

**Owner**: docs

**Files to modify**:
- `README.md` — Phase 3b status, axis/title/legend editing
- `src/lumen/core/commands/CLAUDE.md` — new command files
- `src/lumen/plot/CLAUDE.md` — HitTester extensions, Legend class
- `src/lumen/ui/CLAUDE.md` — three new dialogs
- `docs/reviews/phase-3b-review.md` — at phase end

**Dependencies**: T10 (after features merged)

---

## Parallel Execution Schedule

```
Round 1 (parallel, ~4h):
  Backend:   T1 (Axis setters) + T2 (title state) + T3 (Legend)
  Frontend:  T5 (HitTester regions) + T6 (computeMargins)

Round 2 (~3h):
  Backend:   T4 (bundled commands)
  Frontend:  T6.5 (hitTestPoint + crosshair)

Round 3 (parallel, ~4h):
  Frontend:  T7 (AxisDialog) + T8 (TitleDialog+inline) + T9 (LegendDialog)

Round 4 (~3h):
  Frontend:  T10 (PlotCanvas dispatch + hover highlighting)

Round 5 (parallel, ~3h):
  QA:        T11 (unit tests) + T12 (integration) + T13 (regression)

Round 6 (~1h):
  Integration: T14 (merges, tag)
  Docs:        T15 (README, reviews)

Round 7:
  Human verification (19 acceptance checks + 12 real-data checks)
```

**Total wall time**: ~18-22 hours across rounds.

## Risks

- **Axis as QObject**: making Axis a QObject prevents copying.
  PlotScene already stores xAxis_ and yAxis_ as members (not in
  a vector), so this is safe. Verify no code copies Axis objects.
- **computeMargins jiggle**: live editing may cause margin
  oscillation. 1-pixel debounce threshold mitigates.
- **HitTester region precedence**: overlapping regions (e.g.,
  title rect extends into axis area) resolved by ADR-024 ordering.
- **Inline title editor focus**: Wayland focus handling differs
  from X11. Test on both if available.

## Lessons Applied

- Bundled commands match dialog UX (Phase 3a lesson 1).
- QA exclusively owns test files.
- Frontend PRs must include Phase 3a regression verification.
- Coordinator commits on behalf of permission-blocked agents.
- Build verification with QT_QPA_PLATFORM=offscreen and
  ASAN_OPTIONS=detect_leaks=0 before every commit.
