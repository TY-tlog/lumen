# Phase 3b — Axis, Title, Legend Editing + Dynamic Margins

## Goal

Complete the MATLAB-figure-style editing experience started in
Phase 3a. After this phase, every visible plot element (line,
axis, title, legend) is double-clickable and editable, and plot
margins adjust dynamically to label content. The user can
produce a publication-ready figure entirely inside Lumen.

This phase resolves the last Phase 2 tech debt item, ADR-013
(hardcoded plot margins), as a natural side effect of axis label
editing requiring dynamic layout.

## Active agents

Architect, Backend, Frontend, QA, Integration, Docs.

## User-visible deliverables

### 3b.1 Axis editing

1. Hover over an axis (line, ticks, or label area) highlights it.
2. Double-click an axis opens AxisDialog (non-modal, Apple-mood).
3. AxisDialog provides:
   - Label text
   - Range mode: Auto / Manual
   - Manual range: min and max spinboxes (only when Manual)
   - Tick count: Auto / Manual (3 to 20)
   - Tick format: Auto / Scientific / Fixed N decimals
   - Grid visibility toggle
4. All changes apply immediately within 50 ms.
5. Cancel reverts via CommandBus undo.
6. Right-click axis opens context menu with Edit properties and
   Reset to default.

### 3b.2 Title editing

7. Plot title area is hover-highlighted when editable.
8. Double-click title area enters inline edit mode: a text field
   overlay appears at the title position, pre-filled with current
   text.
9. Enter confirms, Esc cancels.
10. Right-click title opens TitleDialog for font size and weight.
11. If no title is set, double-clicking the area above the plot
    opens an empty inline editor; typing text creates the title.

### 3b.3 Legend editing

12. Hover over legend highlights its border.
13. Double-click legend opens LegendDialog (non-modal):
    - Position: Top-Left / Top-Right / Bottom-Left / Bottom-Right
      / Outside-Right
    - Visible toggle
    - Per-series rows: name (editable), color swatch (read-only,
      reflects current LineSeries color)
14. Position changes apply immediately.
15. Renaming a series in the legend updates the LineSeries name
    via SetSeriesNameCommand (same command class as Phase 3a).

### 3b.4 Dynamic plot margins

16. Plot margins are no longer hardcoded (60/50/30/15). PlotScene
    computes margins from current axis labels, tick label sizes,
    title presence, and legend position using tokens::spacing as
    base units.
17. Editing an axis label that grows in length causes plot area
    to shrink and labels to fit. No clipping.
18. Editing the title from empty to non-empty adds top margin
    space. Removing title removes the margin.
19. Setting legend to Outside-Right adds right margin space for
    the legend.
20. Recomputation is debounced to once per 16 ms during live
    edits to prevent jitter.

## Technical deliverables

### core/commands/AxisCommands.{h,cpp}

Five command classes for axis editing. Each holds a non-owning
Axis pointer and old/new values:

- SetAxisLabelCommand(Axis*, QString old, QString new)
- SetAxisRangeCommand(Axis*, RangeMode oldMode, double oldMin,
  double oldMax, RangeMode newMode, double newMin, double newMax)
- SetAxisTickCountCommand(Axis*, int old, int new)
- SetAxisTickFormatCommand(Axis*, TickFormat old, TickFormat new)
- SetAxisGridVisibleCommand(Axis*, bool old, bool new)

Headers forward-declare Axis; cpp links plot module.

### core/commands/TitleCommand.{h,cpp}

- SetPlotTitleCommand(PlotScene*, QString old, QString new)

### core/commands/LegendCommands.{h,cpp}

Three command classes:

- SetLegendPositionCommand(Legend*, Position old, Position new)
- SetLegendVisibleCommand(Legend*, bool old, bool new)
- SetSeriesNameCommand already exists from Phase 3a; reuse.

### plot/Axis additions (Backend)

Add setters and changed signal to existing Axis class:

- setLabel(QString)
- setRangeMode(RangeMode), setManualRange(double min, double max)
- setTickCount(int)  // 0 = auto
- setTickFormat(TickFormat)  // enum: Auto, Scientific, FixedN(int)
- setGridVisible(bool)
- changed() signal emitted on any setter when value changes

Each setter compares old vs new before emit.

### plot/PlotScene title additions (Backend)

PlotScene currently renders title via PlotRenderer if set. Add:

- setTitle(QString), title()
- setTitleFontPx(int), titleFontPx()
- setTitleWeight(QFont::Weight), titleWeight()
- changed() signal

### plot/Legend additions (Backend)

Phase 2 PlotRenderer draws legend statically. Phase 3b extracts
legend state into a Legend class:

- enum class Position { TopLeft, TopRight, BottomLeft,
  BottomRight, OutsideRight }
- setPosition(Position), position()
- setVisible(bool), isVisible()
- entries() returns list referencing LineSeries (name + color)
- changed() signal

PlotRenderer is updated to read Legend state from PlotScene
instead of hardcoding.

### plot/HitTester extension

Phase 3a HitKind only had None, LineSeries, PlotArea. Phase 3b
adds:

- HitKind::XAxis
- HitKind::YAxis
- HitKind::Title
- HitKind::Legend

hitAt() now identifies these regions:

- XAxis hit: pixel within X axis line, ticks, or label band
- YAxis hit: pixel within Y axis line, ticks, or label band
- Title hit: pixel within title bounding rect
- Legend hit: pixel within legend frame
- LineSeries hit: as in Phase 3a (highest precedence)
- PlotArea: fallback

Region geometry comes from PlotScene::layout() which already
computes plot rect, axis rects, title rect, legend rect.

### plot/InteractionController extension

editRequested signal payload now includes all HitKind values, so
PlotCanvas dispatches to the right dialog:

- HitKind::LineSeries -> LineSeriesDialog (Phase 3a)
- HitKind::XAxis or YAxis -> AxisDialog
- HitKind::Title -> inline title editor (or TitleDialog on
  right-click)
- HitKind::Legend -> LegendDialog

InteractionController adds Mode::EditingTitleInline for the
inline title editor state.

### plot/PlotScene::computeMargins() (resolves ADR-013)

New method:
struct PlotMargins { int left, top, right, bottom; };
PlotMargins computeMargins(const QFontMetrics& fm) const;

Implementation:
- left = max(Y tick label widths) + tokens::spacing::md +
  Y axis label height + tokens::spacing::sm
- top = title height (if title set) + tokens::spacing::md;
  else tokens::spacing::sm
- right = legend width if legend.position == OutsideRight; else
  tokens::spacing::md
- bottom = X tick label height + tokens::spacing::md +
  X axis label height + tokens::spacing::sm

PlotScene::layout() calls computeMargins() instead of using
hardcoded 60/50/30/15. Recomputed on every paint, but the result
is cheap (just QFontMetrics calls).

PlotScene caches the last computed margins and only triggers an
update if margins changed beyond a 1-pixel threshold (debounce
jitter).

### ui/AxisDialog.{h,cpp}

Non-modal QDialog. Constructor:
AxisDialog(lumen::plot::Axis* target,
 lumen::plot::Axis::Orientation orientation, 
lumen::core::CommandBus* commandBus, 
QWidget* parent = nullptr);

Same Apple-mood styling as LineSeriesDialog. Same Cancel-reverts
pattern via CommandBus history tracking.

Controls:
- QLineEdit for label
- QButtonGroup (Auto / Manual) for range mode
- Two QDoubleSpinBox for min/max (visible only in Manual mode)
- QSpinBox for tick count (0 means Auto)
- QComboBox for tick format (Auto / Scientific / Fixed 0..6)
- QCheckBox for grid visibility

### ui/TitleDialog.{h,cpp} and inline editor

TitleDialog (right-click only): font size spinbox, weight combo
(Normal/Medium/Bold).

Inline editor (double-click): a QLineEdit positioned absolutely
at the title rect, with the same font as the title. Frameless.
Loses focus -> applies. Esc -> reverts.

Implementation lives in PlotCanvas (not a separate file) because
it is a transient overlay tied to PlotCanvas geometry.

### ui/LegendDialog.{h,cpp}

Non-modal QDialog with:
- QComboBox for position
- QCheckBox for visibility
- QTableWidget with two columns (Series name editable, Color
  swatch read-only)

Editing a name in the table emits SetSeriesNameCommand via the
shared CommandBus.

### ui/PlotCanvas modifications

- Receive editRequested with all HitKind values
- Open the right dialog or inline editor based on HitKind
- Handle hover highlighting for axes, title, legend in
  paintEvent (subtle 1px border or slight brightness)
- Manage inline title editor lifecycle

### ui/MainWindow modifications

Pass CommandBus to AxisDialog, TitleDialog, LegendDialog
constructors via PlotCanvasDock (already wired in Phase 3a for
LineSeriesDialog; extend the pattern).

## Tests

Target: 161 to at least 200.

Unit tests:
- test_axis_commands.cpp (at least 5 tests, one per command)
- test_title_command.cpp (at least 2 tests)
- test_legend_commands.cpp (at least 3 tests)
- test_axis_setters.cpp (at least 6 tests, one per setter)
- test_plot_scene_title.cpp (at least 3 tests)
- test_legend.cpp (at least 4 tests)
- test_hit_tester_regions.cpp (at least 6 tests:
  XAxis hit, YAxis hit, Title hit, Legend hit, precedence
  ordering, miss returns PlotArea)
- test_plot_scene_margins.cpp (at least 5 tests:
  small font produces smaller margins, long Y label widens
  left margin, title presence increases top margin, OutsideRight
  legend increases right margin, debounce 1-pixel threshold)

Integration tests:
- test_axis_dialog_edit.cpp (open AxisDialog, change label and
  range, verify Axis state and CommandBus history)
- test_title_inline_edit.cpp (double-click title area, type new
  text, press Enter, verify PlotScene title)
- test_legend_dialog_edit.cpp (open LegendDialog, change position
  and rename a series, verify state)
- test_dynamic_margin_no_clip.cpp (set very long Y axis label,
  verify it does not clip outside plot widget)
- test_phase_3a_regression.cpp (run double-click on a line, verify
  LineSeriesDialog still works after HitTester extension and
  PlotCanvas changes)

## ADRs

- ADR-021 Non-modal property dialogs (rationale already
  established in Phase 3a; now formalized as the pattern for all
  4 dialogs; alternatives: modal, dockable inspector panel)
- ADR-022 Dynamic plot margins (resolves ADR-013; alternatives:
  user-configurable margins, content-aware fixed presets)
- ADR-023 Inline title editor as PlotCanvas overlay (vs separate
  TitleDialog only; rationale: matches MATLAB and Prism title
  editing UX)
- ADR-024 HitTester precedence ordering (LineSeries highest, then
  Title, then Legend, then Axis, then PlotArea; alternative:
  z-order based)

Each ADR lists at least two alternatives.

## Architecture updates

docs/architecture.md gets a Phase 3b additions section covering:

- Three new command groups (AxisCommands, TitleCommand,
  LegendCommands)
- Axis, PlotScene title, and Legend setter additions
- HitTester extended to all hit kinds
- InteractionController dispatches to four dialog types
- PlotScene::computeMargins() replaces hardcoded layout (resolves
  ADR-013)
- Inline title editor as PlotCanvas overlay
- Layering invariant unchanged

## Acceptance criteria

Build clean. At least 200 tests pass under ASan and UBSan.

Manual flow:
1. Open electrophysiology CSV
2. Auto line plot appears (Phase 2 behavior preserved)
3. Hover over X axis: axis highlights
4. Double-click X axis: AxisDialog opens
5. Change X label to Time (ms): label updates immediately
6. Set manual range 0.2 to 0.8: plot redraws with new range
7. Cancel: label and range revert
8. Re-open AxisDialog, change label to Time (ms), close
9. Double-click Y axis: AxisDialog opens
10. Change Y label to Vm (mV), close
11. Double-click empty area above plot: inline editor appears
12. Type "Action potential trace", press Enter: title appears
13. Plot top margin grows to fit title
14. Double-click legend: LegendDialog opens
15. Move legend to Outside-Right: legend moves, plot right
    margin grows
16. Rename series in legend: name updates everywhere
17. Ctrl+Z multiple times: each axis/title/legend edit undoes
    individually
18. Phase 3a line editing still works (double-click line opens
    LineSeriesDialog)
19. Pan, wheel zoom, box zoom, double-click reset all still work

## Real-data exit criterion

Owner opens d20251029_s002 (or comparable electrophysiology CSV)
and confirms all of:

- [ ] Edit X axis label to Time (ms) successfully
- [ ] Edit Y axis label to Vm (mV) successfully
- [ ] Set X manual range to a specific spike window
- [ ] Set Y manual range to publication style (e.g. -80 to +40)
- [ ] Set tick format to fixed 1 decimal
- [ ] Edit plot title to cell ID (e.g. d20251029_s002)
- [ ] Move legend to a non-overlapping position
- [ ] Rename the series to something meaningful
- [ ] Combined with Phase 3a line editing, the resulting figure
      is publication-ready: no further edits in Illustrator or
      Inkscape required for axis labels, ranges, title, legend,
      line color, line width
- [ ] All changes survive Ctrl+Z and Ctrl+Shift+Z correctly
- [ ] No regression of Phase 2 (pan/zoom) or Phase 3a (line
      editing)
- [ ] Long axis labels do not clip out of the plot widget

If any fails, Phase 3b is not closed. Failure becomes a task in
Phase 3b plan and must be fixed before Phase 4.

## Non-goals (deferred to Phase 4 or later)

- Edit persistence across sessions (Phase 4)
- Style presets / save figure style (Phase 4)
- Recent colors in color picker (Phase 4)
- Copy/paste style between plots (Phase 4)
- Font family editing (Phase 4)
- Pan/zoom undoable (deliberate)
- Marker editing (Phase 5, scatter prerequisite)
- Scatter / bar / image plots (Phase 5)
- Export to PNG/SVG/PDF (Phase 5)
- Log scale axes (Phase 5)
- Dark theme (Phase 4 or later)
- Data decimation / LOD (Phase 9)

## Risks and mitigations

| Risk | Mitigation |
|---|---|
| HitTester extension breaks Phase 3a line hit detection | test_phase_3a_regression must run before merge; precedence ordering documented in ADR-024 |
| Dynamic margins cause plot to jiggle during live label edit | 1-pixel debounce threshold, documented in ADR-022; visual test |
| Long labels still clip due to font metric mismatch | Use QFontMetrics::horizontalAdvance not boundingRect width; test with long labels |
| Inline title editor focus issues on Wayland | Use QLineEdit setAttribute Qt::WA_TranslucentBackground; test both X11 and Wayland |
| Legend dragging conflicts with pan | Legend hit takes precedence over PlotArea, but only on press inside legend frame; pan still works elsewhere |
| AxisDialog Auto/Manual range mode confuses users | Default to Auto, show Manual fields only when Manual selected; Cancel always reverts |

## Task breakdown

### Architect (S, design only)
- docs/plans/phase-3b-plan.md
- ADR-021, ADR-022, ADR-023, ADR-024
- docs/architecture.md Phase 3b section
- STATUS opening entry

### Backend (M)
- core/commands/AxisCommands.{h,cpp}
- core/commands/TitleCommand.{h,cpp}
- core/commands/LegendCommands.{h,cpp}
- Axis setter additions + changed signal
- PlotScene title state + setter + signal
- Legend class extraction from PlotRenderer
- Unit tests for all commands and setters
- Approximately 6-8 hours

### Frontend (L - largest workload)
- HitTester extension (XAxis, YAxis, Title, Legend kinds)
- HitTester precedence ordering per ADR-024
- InteractionController dispatch to four dialogs
- ui/AxisDialog.{h,cpp}
- ui/TitleDialog.{h,cpp}
- Inline title editor in PlotCanvas
- ui/LegendDialog.{h,cpp}
- PlotScene::computeMargins() implementation
- PlotCanvas hover highlighting for axes, title, legend
- Wire all dialogs to CommandBus via PlotCanvasDock
- Approximately 8-12 hours

### QA (M)
- All unit tests (37+ new)
- All integration tests (5 new)
- test_phase_3a_regression mandatory before any Frontend merge
- Real-data verification at phase end
- Approximately 4-6 hours

### Integration (S, ongoing)
- Merge windows after each task lands
- Update CHANGELOG.md
- Tag vphase-3b at exit

### Docs (S)
- Update README with Phase 3b status and What works list
- Update src/lumen/core/commands/CLAUDE.md (new submodule)
- Update src/lumen/plot/CLAUDE.md (HitTester extension)
- Update src/lumen/ui/CLAUDE.md (3 new dialogs)
- Write docs/reviews/phase-3b-review.md at phase end

## Exit checklist

- Build clean
- At least 200 tests pass under ASan and UBSan
- Real-data exit criterion passed (owner own CSV)
- ADR-013 marked resolved with commit SHA (dynamic margins)
- ADR-021, 022, 023, 024 committed
- Phase 3a regression test passes (line editing still works)
- Phase 2 regression test passes (pan/zoom still work)
- docs/reviews/phase-3b-review.md committed
- vphase-3b tag pushed
- Human used Lumen to complete one publication-style figure
  from real data, end to end (line color/width + axis labels +
  title + legend + manual range)
