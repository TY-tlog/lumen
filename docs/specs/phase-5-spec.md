# Phase 5 — Scatter and Bar Plots

## Goal

Add scatter plot and bar plot support by extracting a PlotItem
abstraction and adding two new implementations alongside
LineSeries. Users pick line, scatter, or bar when adding a
series; all three types participate fully in editing (Phase
3a/3b), saving, and export (Phase 4).

This phase is delivered as two sub-phases with a merge point.
5.1 is refactor-only — LineSeries behavior must be bit-identical
before and after. 5.2 adds the new types.

## Active agents

Architect, Backend, Frontend, QA, Integration, Docs.

## Scope: two internal sub-phases

- Phase 5.1: PlotItem abstraction. LineSeries becomes a PlotItem
  implementation. Zero behavioral change. Human gate before 5.2.
- Phase 5.2: ScatterSeries and BarSeries, property dialogs,
  column picker extension, workspace format extension.

## Phase 5.1 — PlotItem abstraction

### Technical deliverables

#### plot/PlotItem.{h,cpp}

Abstract base class:
class PlotItem {
public:
enum class Type { Line, Scatter, Bar };
virtual ~PlotItem() = default;
virtual Type type() const = 0;
virtual QRectF dataBounds() const = 0;
virtual void paint(QPainter* painter,
                   const CoordinateMapper& mapper,
                   const QRectF& plotArea) const = 0;
virtual std::optional<HitResult> hitTestPoint(
    QPoint pixel,
    const CoordinateMapper& mapper,
    double maxPixelDistance) const = 0;

virtual bool isVisible() const = 0;
virtual QString name() const = 0;
virtual QColor primaryColor() const = 0;  // legend swatch
virtual size_t xColumn() const = 0;
virtual size_t yColumn() const = 0;
};

#### plot/LineSeries refactor

- Inherit from PlotItem
- Move existing paint/hit-test into virtual overrides
- Public API unchanged — existing callers work without mod
- LineSeries-specific setters (setColor, setLineWidth, setStyle,
  etc.) stay on LineSeries, not on PlotItem base

#### plot/PlotScene refactor

- Holds std::vector<std::unique_ptr<PlotItem>> (was LineSeries)
- addItem(std::unique_ptr<PlotItem>) as primary add method
- Existing addSeries(LineSeries) wraps and delegates (backward
  compatibility for callers)
- items() returns const ref to vector
- itemAt(index) returns PlotItem*

#### plot/PlotRenderer adjustment

- Iterates items as PlotItem* and calls virtual paint
- No type dispatch needed — polymorphism handles it
- Legend uses PlotItem::primaryColor() and name()

#### plot/HitTester adjustment

- hitTestPoint iterates PlotItems, calls virtual hitTestPoint
- Double-click dispatch (from Phase 3b) now checks
  PlotItem::type() to pick the right dialog

#### core/io/WorkspaceFile extension (from Phase 4)

- Add "type" field per series entry during save
- On load, missing "type" defaults to "line" (backward compat
  with Phase 4 workspace files)
- Still schema version 1 (additive change)

### Phase 5.1 tests

- test_plot_item_base.cpp — any concrete helper methods on base
- test_line_series_as_plot_item.cpp — LineSeries behavior
  identical after refactor (paint output pixel-equal, hit-test
  returns same results, bounds unchanged)
- test_plot_scene_plot_items.cpp — PlotScene operations work
  through PlotItem base
- Regression: ALL existing 247 tests must pass UNCHANGED

### Phase 5.1 exit gate (human verification)

Before Phase 5.2 starts, human verifies:

- [ ] Build clean, all 247 tests pass unchanged
- [ ] Open a CSV, line plot still renders identically
- [ ] Pan, zoom, crosshair, box zoom, auto-range all behave
      identically
- [ ] Double-click line, edit color/width/style, verify behavior
      identical to Phase 3a/3b
- [ ] Save workspace, reopen, verify identical restoration
- [ ] Export PNG, compare visually to a pre-5.1 export (no
      visible difference expected)

If any behavior differs, Phase 5.1 is not closed. No Phase 5.2
until 5.1 is clean.

## Phase 5.2 — Scatter and Bar

### User-visible deliverables

1. Column picker gains a "Plot type" selector: Line (default) /
   Scatter / Bar
2. Adding a series creates the selected type
3. Double-click a scatter marker opens ScatterPropertyDialog
4. Double-click a bar opens BarPropertyDialog
5. Scatter properties: color, marker shape (circle / square /
   triangle / diamond / plus / cross), marker size (3-20 px),
   filled / outlined-only, visible, name
6. Bar properties: fill color, outline color (with "none"
   option), bar width (relative 0.1-1.0 of median X spacing),
   visible, name
7. Legend draws type-appropriate swatches: line segment for
   line, filled marker for scatter, filled rectangle for bar
8. Crosshair snaps to nearest marker center for scatter series
9. Crosshair does not apply to bar series (not meaningful)
10. Save workspace preserves series type and type-specific
    properties
11. Reopening a workspace with mixed line/scatter/bar restores
    all types correctly
12. Export (PNG/SVG/PDF) renders all three types

### Technical deliverables

#### plot/ScatterSeries.{h,cpp}
enum class MarkerShape {
Circle, Square, Triangle, Diamond, Plus, Cross
};
class ScatterSeries : public PlotItem {
public:
ScatterSeries(DataFrame* df, size_t xCol, size_t yCol,
QColor color, QString name);
Type type() const override { return Type::Scatter; }
void paint(QPainter*, const CoordinateMapper&,
           const QRectF&) const override;
std::optional<HitResult> hitTestPoint(
    QPoint, const CoordinateMapper&, double) const override;
QRectF dataBounds() const override;

// Setters (each emits changed())
void setColor(QColor);
void setMarkerShape(MarkerShape);
void setMarkerSize(int pixels);  // 3-20
void setFilled(bool);
void setName(QString);
void setVisible(bool);

// Getters omitted for brevity
signals:
void changed();
};

Paint: iterate (x,y) pairs, skip NaN, transform to pixels, draw
marker at each pixel location. Clip to plot area. Marker shapes
rendered via QPainter primitives (no QIcon/SVG).

Hit-test: iterate samples, find closest marker center within
maxPixelDistance. Return sample index as HitResult.

#### plot/BarSeries.{h,cpp}
class BarSeries : public PlotItem {
public:
BarSeries(DataFrame* df, size_t xCol, size_t yCol,
QColor fill, QString name);
Type type() const override { return Type::Bar; }
void paint(QPainter*, const CoordinateMapper&,
           const QRectF&) const override;
std::optional<HitResult> hitTestPoint(
    QPoint, const CoordinateMapper&, double) const override;
QRectF dataBounds() const override;  // includes y=0 baseline

void setFillColor(QColor);
void setOutlineColor(QColor);  // default transparent = none
void setBarWidth(double relative);  // 0.1-1.0
void setName(QString);
void setVisible(bool);
signals:
void changed();
};

Paint: iterate (x,y), skip NaN, for each bar compute rect
[x - width/2, 0] to [x + width/2, y] in data space, transform
to pixels, draw filled rect with optional outline.

Bar width: width = relativeWidth * medianXSpacing, with a
minimum of 2 pixels to prevent disappearance at high zoom-out.

Hit-test: point-in-rectangle check for each bar.

#### core/commands/ScatterCommands.{h,cpp}

Bundled command (matches Phase 3a ChangeLineStyleCommand
pattern):

- ChangeScatterPropertiesCommand captures color + markerShape +
  markerSize + filled + name + visible as one undoable unit

#### core/commands/BarCommands.{h,cpp}

- ChangeBarPropertiesCommand captures fillColor + outlineColor +
  barWidth + name + visible as one undoable unit

#### ui/ScatterPropertyDialog.{h,cpp}

Non-modal, Apple-mood styled. Reuses ColorPicker from Phase 3a.
Marker shape combo with icon previews (drawn at combo population
time, not SVG assets). Size spinbox 3-20 px. Filled/outlined
toggle. Name QLineEdit, visible QCheckBox. Cancel reverts via
CommandBus session tracking (same pattern as
LinePropertyDialog).

#### ui/BarPropertyDialog.{h,cpp}

Non-modal. Fill color picker, outline color picker with "None"
checkbox, bar width slider (0.1-1.0), name, visible.

#### ui/PlotCanvasDock column picker update

Add QComboBox above the existing X/Y column combos:

  Plot type: [Line ▾]  (Line / Scatter / Bar)

"Add series" creates the series of the selected type with the
next palette color.

#### plot/HitTester double-click dispatch

Extend the Phase 3b dispatch logic: after identifying the hit
PlotItem, check PlotItem::type() and open:
- Type::Line → LinePropertyDialog
- Type::Scatter → ScatterPropertyDialog
- Type::Bar → BarPropertyDialog

#### plot/PlotRenderer legend swatches

Draw type-appropriate swatch per legend row:
- Line: 20-pixel line segment in series color and style
- Scatter: filled marker (of series shape and size) in color
- Bar: filled 12x8 rectangle in fill color with optional outline

Row height fixed at 18 px regardless of type.

#### core/io/WorkspaceFile series type-specific fields

Series entry in workspace JSON carries type-specific fields:
{
"type": "scatter",
"xColumn": 0, "yColumn": 2,
"color": "#0a84ff",
"markerShape": "circle",
"markerSize": 6,
"filled": true,
"name": "FI curve",
"visible": true
}
{
"type": "bar",
"xColumn": 0, "yColumn": 3,
"fillColor": "#30d158",
"outlineColor": null,
"barWidth": 0.8,
"name": "Mean response",
"visible": true
}

Load: dispatch on "type" field, construct the matching PlotItem
subclass, apply fields.

### Phase 5.2 tests

Unit tests (>=20 new):
- test_scatter_series.cpp (>=6): paint correctness on known
  data, hit-test returns nearest marker, NaN skipped, bounds
  correctness, each setter triggers changed(), visible=false
  hides rendering
- test_bar_series.cpp (>=6): paint with computed bar width,
  hit-test point-in-rect, NaN skipped, zero-baseline in bounds,
  setters, degenerate single-point fallback width
- test_scatter_commands.cpp (>=2): do applies, undo restores,
  bundled semantics
- test_bar_commands.cpp (>=2): same
- test_workspace_mixed_types.cpp: save line+scatter+bar, load,
  verify all three restored with correct fields
- test_column_picker_plot_type.cpp: each type creates the
  correct PlotItem subclass
- test_legend_mixed_swatches.cpp: legend row heights and swatch
  rendering for all 3 types

Integration tests (>=4 new):
- test_double_click_scatter_edit.cpp: double-click marker opens
  ScatterPropertyDialog, change marker shape, verify
- test_double_click_bar_edit.cpp: double-click bar opens
  BarPropertyDialog, change fill, verify
- test_export_mixed_types.cpp: export mixed plot to PNG, verify
  all three types present (heuristic check on pixel colors)
- test_phase_3a_regression.cpp (updated): line editing still
  works after hit-test dispatch extension

Target: 247 → at least 290.

## ADRs

- ADR-028 PlotItem abstraction design
  Decision: abstract base class with virtual paint / hit-test /
  bounds / type(). Alternatives: std::variant-based dispatch
  (rejected: no polymorphism for future types), separate
  renderer per type with PlotItem as pure data (rejected:
  scatters rendering logic, hurts readability).

- ADR-029 Series type immutable after creation
  Decision: user picks type at creation in column picker.
  Changing a series's type requires delete + re-add.
  Alternatives: mutable via property dialog (rejected: complex
  state migration — what happens to line width when converting
  to bar?), automatic type inference from data (rejected: takes
  control away from user).

- ADR-030 Marker rendering strategy
  Decision: fixed palette of 6 shapes rendered via QPainter
  primitives; marker size in pixels not data units.
  Alternatives: SVG or QIcon markers (rejected: DPI
  inconsistency, overkill), data-unit sizing (rejected: markers
  shrink when zooming out, ugly).

- ADR-031 Bar layout
  Decision: single series per bar position, width as fraction
  of median X spacing, pixel minimum of 2 px to prevent
  disappearance. Alternatives: grouped bars (deferred — not
  needed for Phase 5 use cases), stacked bars (deferred), fixed
  pixel width (rejected: doesn't scale with data).

Each ADR lists at least two alternatives.

## Architecture updates

docs/architecture.md Phase 5 section covering:
- plot/PlotItem abstract base in the plot layer
- LineSeries, ScatterSeries, BarSeries as concrete
  implementations
- PlotScene now holds PlotItem*; PlotRenderer and HitTester
  work through polymorphism
- New commands: core/commands/ScatterCommands,
  core/commands/BarCommands
- Workspace file format extended with "type" discriminator per
  series (still schema v1, backward compat)
- Double-click dispatch extended to 3 dialog types
- Layer invariant unchanged

## Acceptance criteria
./scripts/dev_build.sh       # zero warnings, zero errors
./scripts/dev_test.sh        # all tests pass under ASan+UBSan
./scripts/dev_run.sh         # manual flow below

Manual flow:

Phase 5.1 (refactor-only):
1. Open a real electrophysiology CSV
2. Auto line plot renders — visually identical to pre-5.1
3. Pan, wheel zoom, box zoom, double-click reset: identical
4. Crosshair: identical
5. Double-click line → LinePropertyDialog → edit → undo: all
   identical to Phase 3a behavior
6. Save workspace, reopen: identical restoration
7. Export PNG: visually identical to pre-5.1 export

Phase 5.2 (new features):
8. Open a CSV with at least 4 numeric columns
9. Column picker: Line type, pick (X, Y1), add → line appears
10. Column picker: Scatter type, pick (X, Y2), add → scatter
    markers overlay in palette color 1
11. Column picker: Bar type, pick (X, Y3), add → bars overlay
    in palette color 2
12. Double-click a scatter marker → ScatterPropertyDialog opens
13. Change marker shape to Diamond, size to 10 → immediate
14. Change fill to outlined-only → markers become hollow
15. Double-click a bar → BarPropertyDialog opens
16. Change fill color → immediate update
17. Set outline color to black → bars get outline
18. Ctrl+Z several times: each edit undoes in reverse order
19. Save workspace (Ctrl+S)
20. Close document, reopen → all three series restored with
    correct types, colors, shapes, sizes
21. Export as PDF → open PDF, verify line + scatter + bars all
    rendered

Regression:
- [ ] Phase 2: pan/zoom/crosshair
- [ ] Phase 3a: line property editing
- [ ] Phase 3b: axis/title/legend editing
- [ ] Phase 4: save workspace, export

## Real-data exit criterion

Owner opens a CSV with data suitable for multiple plot types
and confirms all of:

- [ ] Creates a line plot from one pair (e.g. voltage trace)
- [ ] Creates a scatter plot from another pair (e.g. FI curve
      or dose-response)
- [ ] Creates a bar plot from another pair (e.g. mean values
      per condition)
- [ ] Edits each to publication style
- [ ] Saves workspace; reopening restores all three
- [ ] Exports as PDF
- [ ] The exported PDF is usable in a manuscript without
      further tool work

## Non-goals (deferred)

- Grouped or stacked bars (later)
- Error bars on any type (later)
- Line-with-markers style (line and scatter are separate types
  in Phase 5)
- Changing series type after creation (per ADR-029)
- Box plots, violin plots, heatmaps, image plots (later)
- Custom marker shapes beyond the 6 defaults
- Categorical X axis for bar charts (numeric X only in Phase 5)
- Log-scale axes
- Dark theme (Phase 6)
- Multi-plot / subplot (Phase 7)

## Risks and mitigations

| Risk | Mitigation |
|---|---|
| Phase 5.1 refactor changes LineSeries behavior | All 247 tests must pass unchanged; Phase 5.1 exit gate is human verification of Phase 2-4 flows |
| Phase 4 workspace files fail to load after extension | Missing "type" defaults to "line"; test with an actual pre-5.1 workspace file |
| HitTester dispatch breaks Phase 3a line double-click | test_phase_3a_regression updated and mandatory before Frontend merge |
| ScatterSeries with 100k+ points is slow | Document as known limit; Phase 9 LOD deferred |
| Bar width computation degenerate on single data point | Fallback to 2px minimum; unit test |
| Marker rendering differs between PNG and SVG export | Single PlotRenderer code path; test mixed-type export |
| Legend layout breaks with 3 swatch types | Fixed 18 px row height; test with 3+ series of mixed types |
| ColumnPicker "Plot type" combo confuses users | Default to Line; only shows when adding, not when editing |

## Task breakdown

### Phase 5.1 — refactor sub-phase

- T1 Backend: PlotItem abstract base + LineSeries refactor to
  inherit. Pure refactor. M. ~3h.
- T2 Backend: PlotScene refactor to hold PlotItem*. Keep
  addSeries(LineSeries) as compat wrapper. M. ~2h.
- T3 Backend: PlotRenderer polymorphic dispatch. HitTester
  adjustment. S. ~1h.
- T4 Backend: WorkspaceFile type field with backward-compat
  load. S. ~1h.
- T5 QA: regression — all 247 tests pass unchanged. S. ~1h.
- M5.1 merge gate: human verifies Phase 2-4 behaviors identical.

### Phase 5.2 — new types sub-phase

- T6 Backend: ScatterSeries + unit tests. M. ~3h.
- T7 Backend: BarSeries + unit tests. M. ~3h.
- T8 Backend: ScatterCommands + BarCommands (bundled). S. ~1h.
- T9 Frontend: ScatterPropertyDialog (reuses ColorPicker). S.
  ~2h.
- T10 Frontend: BarPropertyDialog. S. ~2h.
- T11 Frontend: ColumnPicker "Plot type" combo. S. ~1h.
- T12 Frontend: HitTester dispatch extension for scatter/bar
  dialogs. S. ~1h.
- T13 Frontend: PlotRenderer legend with 3 swatch types. S.
  ~1h.
- T14 QA: scatter/bar unit tests (>=14), integration tests
  (>=4), regression including test_phase_3a_regression update.
  M. ~3h.
- M5.2 merge gate: real-data verification.

### Closing

- T15 Docs: README update, src/lumen/core/commands/CLAUDE.md
  update, src/lumen/plot/CLAUDE.md update, docs/reviews/
  phase-5-review.md WRITTEN AND COMMITTED IN THE SAME COMMIT AS
  THE CLOSING .lumen-ops/STATUS.md ENTRY. This is the Phase
  3b/4 lesson — do not split review and closing into separate
  commits.

## Exit checklist

Phase 5.1 sub-checklist:
- [ ] Build clean, 0 warnings
- [ ] All 247 existing tests pass unchanged
- [ ] Human verifies Phase 2-4 flows behave identically
- [ ] No visual diff on known plot renders
- [ ] Pre-5.1 workspace file loads correctly

Phase 5.2 sub-checklist:
- [ ] All Phase 5.2 tests pass (>=20 new unit, >=4 new
      integration)
- [ ] Total test count >= 290
- [ ] Human creates line + scatter + bar in one plot, edits
      each, saves, reopens, exports PDF
- [ ] Legend renders 3 swatch types correctly
- [ ] ColumnPicker type combo works for all 3 types

Phase 5 overall:
- [ ] Phase 2, 3a, 3b, 4 all regression-clean
- [ ] ADR-028, 029, 030, 031 committed
- [ ] docs/reviews/phase-5-review.md committed IN THE SAME
      COMMIT as the closing STATUS entry
- [ ] vphase-5 tag pushed
