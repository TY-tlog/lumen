# Phase 5 Plan — Scatter and Bar Plots with PlotItem Abstraction

> Reference: `docs/specs/phase-5-spec.md`

## Hard rules

1. **Review-in-same-commit**: docs/reviews/phase-5-review.md MUST
   be written and committed in the SAME COMMIT as the closing
   .lumen-ops/STATUS.md entry. This rule appears verbatim in T15's
   task description. Phase 3b and Phase 4 proved that burying this
   in the exit checklist is insufficient.

2. **Phase 5.1 is refactor-only**: LineSeries behavior must be
   bit-identical before and after. ALL 247 existing tests must
   pass UNCHANGED. M5.1 gate is non-negotiable.

3. **Single rendering code path** (ADR-026): new plot types render
   through PlotRenderer via polymorphic PlotItem::paint() calls.

4. **Workspace backward compat** (ADR-025): a Phase 4 workspace
   file (no "type" field) must load correctly, defaulting to "line".
   T5 includes a specific test for this.

---

## Task Dependency Graph

```
Phase 5.1 — Refactor:

  ┌─────────────┐
  │T1 PlotItem  │
  │ base class  │
  │ (backend)   │
  └──────┬──────┘
         │
    ┌────┼────────────┐
    ▼    ▼            ▼
  ┌────┐ ┌─────────┐ ┌──────────┐
  │T2  │ │T3 Rend- │ │T4 Work-  │
  │Scn │ │erer +   │ │space type│
  │ref │ │HitTest  │ │field     │
  │    │ │         │ │          │
  └──┬─┘ └────┬────┘ └────┬─────┘
     │        │           │
     └────┬───┘───────────┘
          ▼
    ┌───────────┐
    │T5 QA      │
    │ regression│
    └─────┬─────┘
          ▼
    ═══════════════
    ║ M5.1 GATE  ║  ← human verifies Phase 2-4 identical
    ═══════════════

Phase 5.2 — New Types:

          │
    ┌─────┼─────┐
    ▼           ▼
  ┌──────┐  ┌──────┐   ┌──────────┐
  │T6    │  │T7    │   │T11 Col-  │
  │Scatt │  │Bar   │   │umnPicker │
  │(back)│  │(back)│   │type combo│
  └──┬───┘  └──┬───┘   │(frontend)│
     │         │        └────┬─────┘
     └────┬────┘             │
          ▼                  │
    ┌───────────┐            │
    │T8 Scatter │            │
    │+ Bar cmds │            │
    │(backend)  │            │
    └─────┬─────┘            │
          │                  │
    ┌─────┼──────┐           │
    ▼     ▼      ▼           │
  ┌────┐ ┌────┐ ┌─────┐     │
  │T9  │ │T10 │ │T13  │     │
  │Sca │ │Bar │ │Leg- │     │
  │Dlg │ │Dlg │ │end  │     │
  └──┬─┘ └──┬─┘ └──┬──┘     │
     │      │      │         │
     └──┬───┘──────┘─────────┘
        ▼
  ┌───────────┐
  │T12 Hit-   │
  │Tester     │
  │dispatch   │
  └─────┬─────┘
        ▼
  ┌───────────┐
  │T14 QA     │
  │scatter/bar│
  └─────┬─────┘
        ▼
  ═══════════════
  ║ M5.2 GATE  ║  ← human creates mixed plot, saves, exports
  ═══════════════
        │
        ▼
  ┌───────────┐
  │T15 Docs   │
  │ review +  │
  │ STATUS    │
  │ SAME      │
  │ COMMIT    │
  └───────────┘
```

## Critical path

T1 → T2+T3+T4 → T5 → M5.1 → T6+T7 → T8 → T9+T10 → T12 → T14 → M5.2 → T15

---

## Phase 5.1 — Refactor (Zero Behavioral Change)

### T1 — PlotItem Abstract Base + LineSeries Refactor (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/PlotItem.h`

**Files to modify**:
- `src/lumen/plot/LineSeries.h` / `.cpp` — inherit from PlotItem,
  implement virtual overrides
- `src/lumen/plot/CMakeLists.txt` — add PlotItem.h

**PlotItem interface**:
```cpp
class PlotItem {
public:
    enum class Type { Line, Scatter, Bar };
    virtual ~PlotItem() = default;
    virtual Type type() const = 0;
    virtual QRectF dataBounds() const = 0;
    virtual void paint(QPainter* painter,
                       const CoordinateMapper& mapper,
                       const QRectF& plotArea) const = 0;
    virtual bool isVisible() const = 0;
    virtual QString name() const = 0;
    virtual QColor primaryColor() const = 0;
};
```

LineSeries changes:
- Add `: public PlotItem` inheritance
- Implement type() → Type::Line
- Move the series-drawing code from PlotRenderer into
  LineSeries::paint() (self-rendering pattern)
- dataBounds() wraps existing dataRange()
- primaryColor() returns style().color
- Existing public API (setStyle, setName, buildPolylines, etc.)
  stays on LineSeries, not on PlotItem base

**CRITICAL**: No behavioral change. LineSeries::paint() must
produce pixel-identical output to the current PlotRenderer series
drawing code.

**Acceptance criteria**:
- LineSeries compiles as PlotItem subclass
- All 247 existing tests pass UNCHANGED
- No new tests needed (T5 adds them)

**Dependencies**: none (day 1)

---

### T2 — PlotScene Refactor (Backend, Size: M)

**Owner**: backend

**Files to modify**:
- `src/lumen/plot/PlotScene.h` / `.cpp`

**Changes**:
- Internal storage: `std::vector<std::unique_ptr<PlotItem>>` items_
  (was `std::vector<LineSeries> series_`)
- `void addItem(std::unique_ptr<PlotItem> item)` — primary add
- `void addSeries(LineSeries series)` — backward compat wrapper:
  wraps in unique_ptr, calls addItem
- `const std::vector<std::unique_ptr<PlotItem>>& items() const`
- `PlotItem* itemAt(std::size_t index)` — mutable access
- `const PlotItem* itemAt(std::size_t index) const`
- `std::size_t itemCount() const`
- `void clearItems()`
- Keep `seriesCount()` and `series()` as deprecated compat if
  needed by existing callers, OR update all callers to use items()

autoRange: iterate items, call dataBounds() on each visible item.

**CRITICAL**: All existing callers (PlotRenderer, HitTester,
PlotCanvasDock, WorkspaceFile, commands) must be updated to use
items()/itemAt() instead of series()/seriesAt(). This is the
largest mechanical change in 5.1.

**Acceptance criteria**:
- PlotScene holds PlotItem pointers
- All 247 existing tests pass UNCHANGED
- addSeries(LineSeries) still works for backward compat

**Dependencies**: T1 (PlotItem base must exist)

---

### T3 — PlotRenderer + HitTester Polymorphic (Backend, Size: S)

**Owner**: backend

**Files to modify**:
- `src/lumen/plot/PlotRenderer.cpp` — iterate items as PlotItem*,
  call item->paint() instead of inline series drawing code
- `src/lumen/plot/HitTester.cpp` — iterate items as PlotItem*

**PlotRenderer changes**:
- Remove the series drawing loop that builds polylines and calls
  drawPolyline. Replace with:
  ```cpp
  for (const auto& item : scene.items()) {
      if (!item->isVisible()) continue;
      item->paint(&painter, mapper, plotArea);
  }
  ```
- Legend: use item->primaryColor() and item->name()

**HitTester changes**:
- hitTest: now works on PlotItem. LineSeries's hitTestPoint is
  the existing segment-distance logic.
- hitTestPoint: iterate items, for LineSeries use existing binary
  search logic. Phase 5.2 will add scatter/bar paths.

**Acceptance criteria**:
- Rendering is polymorphic via PlotItem::paint()
- All 247 existing tests pass UNCHANGED
- Visual output identical

**Dependencies**: T1, T2

---

### T4 — WorkspaceFile Type Field (Backend, Size: S)

**Owner**: backend

**Files to modify**:
- `src/lumen/core/io/WorkspaceFile.cpp`

**Changes**:
- On save: add `"type": "line"` to each series entry
- On load: read "type" field. If missing, default to "line"
  (backward compat with Phase 4 workspace files)
- Phase 5.2 will add "scatter" and "bar" dispatch

**Acceptance criteria**:
- Save produces "type":"line" in each series
- Phase 4 workspace file (no "type") loads correctly
- All 247 existing tests pass

**Dependencies**: T2 (PlotScene API change)

---

### T5 — QA Regression + Phase 5.1 Tests (QA, Size: M)

**Owner**: qa

**Files to create**:
- `tests/unit/test_line_series_as_plot_item.cpp`
- `tests/unit/test_workspace_v4_loads.cpp`

**Hard requirement**: ALL 247 existing tests pass UNCHANGED. Do not
modify any existing test file.

**New tests**:
- test_line_series_as_plot_item.cpp (≥4 tests):
  - LineSeries::type() returns Type::Line
  - LineSeries::paint() produces non-empty render (QImage)
  - LineSeries::dataBounds() matches old dataRange()
  - LineSeries::primaryColor() matches style().color
- test_workspace_v4_loads.cpp (≥2 tests):
  - Create a workspace JSON file WITHOUT "type" fields (Phase 4
    format). Load it. Verify all series load as LineSeries.
  - Create a workspace JSON with "type":"line". Load. Same result.

**Acceptance criteria**:
- 247 existing tests pass UNCHANGED
- 6+ new tests pass
- Total ≥ 253

**Dependencies**: T1, T2, T3, T4

---

### M5.1 — Human Verification Gate (BLOCKING)

**NOT a task — a gate. T6 CANNOT start until M5.1 passes.**

Human verifies with real electrophysiology CSV:
1. Build clean, all tests pass
2. Line plot renders identically to pre-5.1
3. Pan, zoom, crosshair, box zoom identical
4. Double-click line → LinePropertyDialog → edit → undo: identical
5. Save workspace, reopen: identical
6. Export PNG: visually identical to pre-5.1

**If any behavior differs**: fix before starting Phase 5.2.

---

## Phase 5.2 — Scatter and Bar

### T6 — ScatterSeries (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/ScatterSeries.h` / `.cpp`
- `tests/unit/test_scatter_series.cpp`

**ScatterSeries : public PlotItem**:
- MarkerShape enum (Circle, Square, Triangle, Diamond, Plus, Cross)
- Stores: color, markerShape, markerSize (3-20px), filled, name,
  visible, Column* xCol, Column* yCol
- paint(): iterate (x,y), skip NaN, map to pixel, draw marker via
  QPainter primitives. Clip to plotArea.
- dataBounds(): min/max of x/y ignoring NaN, includes all points
- primaryColor(): returns color
- type(): returns Type::Scatter

**Tests** (≥6): paint non-empty, hit-test nearest marker, NaN
skipped, bounds correct, setters emit changed(), visible=false
hides.

**Dependencies**: M5.1 gate

---

### T7 — BarSeries (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/BarSeries.h` / `.cpp`
- `tests/unit/test_bar_series.cpp`

**BarSeries : public PlotItem**:
- Stores: fillColor, outlineColor (transparent = none), barWidth
  (relative 0.1-1.0), name, visible, Column* xCol, Column* yCol
- paint(): iterate (x,y), skip NaN, compute rect [x-w/2, 0] to
  [x+w/2, y] in data space, map to pixels, draw filled rect +
  optional outline. Bar width = relativeWidth * medianXSpacing,
  min 2px.
- dataBounds(): includes y=0 baseline
- primaryColor(): returns fillColor
- type(): returns Type::Bar

**Tests** (≥6): paint with bars, hit-test point-in-rect, NaN
skipped, zero-baseline in bounds, single-point fallback width,
setters.

**Dependencies**: M5.1 gate

---

### T8 — ScatterCommands + BarCommands (Backend, Size: S)

**Owner**: backend

**Files to create**:
- `src/lumen/core/commands/ChangeScatterPropertiesCommand.h` / `.cpp`
- `src/lumen/core/commands/ChangeBarPropertiesCommand.h` / `.cpp`
- `tests/unit/test_scatter_commands.cpp`
- `tests/unit/test_bar_commands.cpp`

Bundled pattern (from ChangeLineStyleCommand):
- ChangeScatterPropertiesCommand: color + markerShape + markerSize
  + filled + name + visible
- ChangeBarPropertiesCommand: fillColor + outlineColor + barWidth
  + name + visible

**Tests** (≥2 each): execute applies, undo restores.

**Dependencies**: T6, T7 (setters must exist)

---

### T9 — ScatterPropertyDialog (Frontend, Size: S)

**Owner**: frontend

**Files to create**:
- `src/lumen/ui/ScatterPropertyDialog.h` / `.cpp`

Non-modal QDialog: color button, marker shape combo (6 options with
QPainter-drawn preview icons), size spinbox (3-20), filled checkbox,
name edit, visible checkbox, OK/Cancel.

**Dependencies**: T8 (command)

---

### T10 — BarPropertyDialog (Frontend, Size: S)

**Owner**: frontend

**Files to create**:
- `src/lumen/ui/BarPropertyDialog.h` / `.cpp`

Non-modal QDialog: fill color button, outline color button with
"None" checkbox, bar width slider (0.1-1.0), name edit, visible
checkbox, OK/Cancel.

**Dependencies**: T8 (command)

---

### T11 — ColumnPicker Plot Type Combo (Frontend, Size: S)

**Owner**: frontend

**Files to modify**:
- `src/lumen/ui/PlotCanvasDock.h` / `.cpp`

Add QComboBox "Plot type: Line / Scatter / Bar" in the toolbar
above X/Y combos. "Add Series" creates the selected type with next
palette color. Default: Line.

**Dependencies**: T1 (PlotItem::Type enum)

---

### T12 — HitTester Dispatch Extension (Frontend, Size: S)

**Owner**: frontend

**Files to modify**:
- `src/lumen/ui/PlotCanvasDock.cpp` — onSeriesDoubleClicked
  checks PlotItem::type() and opens matching dialog

**Dispatch**:
```
Type::Line → LinePropertyDialog (Phase 3a, unchanged)
Type::Scatter → ScatterPropertyDialog
Type::Bar → BarPropertyDialog
```

**Dependencies**: T9, T10, T11

---

### T13 — Legend Mixed Swatches (Frontend, Size: S)

**Owner**: frontend

**Files to modify**:
- `src/lumen/plot/PlotRenderer.cpp` — legend drawing

**Swatches per type**:
- Line: 20px line segment in series color and pen style
- Scatter: filled marker of series shape and size
- Bar: filled 12x8 rectangle in fill color + optional outline

Row height: fixed 18px regardless of type.

**Dependencies**: T6, T7 (need ScatterSeries/BarSeries to exist)

---

### T14 — QA Phase 5.2 Tests (QA, Size: M)

**Owner**: qa

**Files to create**:
- `tests/unit/test_scatter_series.cpp` (if not done by T6)
- `tests/unit/test_bar_series.cpp` (if not done by T7)
- `tests/unit/test_scatter_commands.cpp` (if not done by T8)
- `tests/unit/test_bar_commands.cpp` (if not done by T8)
- `tests/unit/test_workspace_mixed_types.cpp`
- `tests/unit/test_legend_mixed_swatches.cpp`
- `tests/integration/test_double_click_scatter_edit.cpp`
- `tests/integration/test_double_click_bar_edit.cpp`
- `tests/integration/test_export_mixed_types.cpp`

**Regression**: all prior tests pass. Phase 3a line editing works.

**Target**: ≥290 total tests.

**Dependencies**: T6-T13

---

### M5.2 — Human Verification Gate

Human creates mixed plot (line + scatter + bar), edits each, saves
workspace, reopens, exports PDF. All types visible and correct.

---

### T15 — Docs Closing (REVIEW AND STATUS IN SAME COMMIT)

**Owner**: docs

**HARD RULE (verbatim from Phase 3b/4 lesson)**: docs/reviews/
phase-5-review.md MUST be WRITTEN AND COMMITTED IN THE SAME COMMIT
as the closing .lumen-ops/STATUS.md entry. This is one commit, one
`git add`, one `git commit`. Not a separate task, not a follow-up.
The coordinator must enforce this. If the review is missing from
the commit, the commit must be amended to include it before push.

**Files in the ONE commit**:
- `docs/reviews/phase-5-review.md`
- `.lumen-ops/STATUS.md` (closing entry)
- `README.md` (update capabilities)
- `src/lumen/plot/CLAUDE.md` (PlotItem, ScatterSeries, BarSeries)
- `src/lumen/core/commands/CLAUDE.md` (ScatterCommands, BarCommands)

After commit: `git tag vphase-5 && git push origin main --tags`.

**Dependencies**: M5.2 gate

---

## Parallel Execution Schedule

```
Phase 5.1:
  Round 1 (~4h):
    Backend: T1 (PlotItem base + LineSeries refactor)

  Round 2 (~3h):
    Backend: T2 (PlotScene) + T3 (Renderer+HitTester) + T4 (Workspace)

  Round 3 (~2h):
    QA: T5 (regression + new tests)

  Gate M5.1: Human verifies Phase 2-4 identical

Phase 5.2:
  Round 4 (parallel, ~4h):
    Backend: T6 (ScatterSeries) + T7 (BarSeries)
    Frontend: T11 (ColumnPicker type combo)

  Round 5 (~2h):
    Backend: T8 (ScatterCommands + BarCommands)

  Round 6 (parallel, ~3h):
    Frontend: T9 (ScatterDialog) + T10 (BarDialog) + T13 (Legend swatches)

  Round 7 (~2h):
    Frontend: T12 (HitTester dispatch)

  Round 8 (~3h):
    QA: T14 (scatter/bar tests)

  Gate M5.2: Human verification

Closing:
  Round 9 (~30min):
    Docs: T15 (review+STATUS SAME commit, then tag)
```

**Total wall time**: ~20-24 hours across rounds.

## Risks

- **PlotScene API change breadth**: T2 touches every caller of
  series()/seriesAt(). Careful mechanical replacement needed.
- **LineSeries::paint() pixel equality**: moving rendering code
  from PlotRenderer to LineSeries must produce identical output.
  Test with QImage comparison.
- **Phase 4 workspace backward compat**: must test with actual
  pre-5.1 workspace file, not just a hand-crafted JSON.
- **Bar width on degenerate data**: single-point data has no X
  spacing. Fallback to 2px minimum.

## Lessons Applied

- Review in same commit as STATUS (Phase 3b/4).
- M5.1 gate blocks Phase 5.2 (Phase 4 two-sub-phase pattern).
- QA exclusively owns test files.
- FigureExporter stub pattern for parallel dev (Phase 4).
- Coordinator commits on behalf of permission-blocked agents.
