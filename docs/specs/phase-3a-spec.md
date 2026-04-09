# Phase 3a — Line Property Editing

## Goal

Allow the user to customize the visual properties of each line
series in a plot: color, line width, line style (solid/dashed/
dotted), and series name. Double-clicking a line opens a property
dialog. This is the first step toward the full property inspector
(Phase 5).

## Context

Phase 2 delivers auto-assigned line styles via PlotStyle::fromPalette.
The user cannot change them. For electrophysiology analysis, the user
often needs to:
- Distinguish overlapping traces by changing color or dash style
- Make a specific trace thicker for emphasis (e.g., voltage vs time)
- Rename series labels for publication-quality legends
- Hide a series temporarily without removing it from the picker

## Dependencies on Phase 2

- `lumen::plot::PlotStyle` — color, lineWidth, penStyle
- `lumen::plot::LineSeries` — holds PlotStyle, name
- `lumen::plot::PlotScene` — owns series vector
- `lumen::ui::PlotCanvas` — renders via PlotRenderer
- `lumen::ui::PlotCanvasDock` — manages PlotScene, column picker

## Deliverables

### D1 — LinePropertyDialog (`src/lumen/ui/`)

A modal `QDialog` for editing one LineSeries's visual properties:

- **Color picker**: QPushButton showing current color, clicks opens
  QColorDialog.
- **Line width**: QDoubleSpinBox (range 0.5–10.0, step 0.5,
  default from PlotStyle).
- **Line style**: QComboBox with Solid, Dash, Dot, DashDot,
  DashDotDot. Shows a small preview line in each item.
- **Series name**: QLineEdit (editable label for legend).
- **Visible**: QCheckBox to show/hide the series.
- **OK / Cancel** buttons. Changes apply on OK only.

Returns a modified `PlotStyle` + name + visibility.

### D2 — Hit-Testing for Line Series (`src/lumen/plot/`)

To double-click a line, we need to know which line the cursor is
near. Implement basic hit-testing:

- `HitTester` class: given a PlotScene, CoordinateMapper, and a
  pixel position (QPointF), finds the nearest LineSeries within a
  tolerance (default 5 pixels).
- Algorithm: for each series, iterate its polylines, compute
  distance from the cursor pixel to each line segment. Return the
  series with minimum distance if within tolerance.
- Returns: `std::optional<int>` (series index) or nullopt.

### D3 — Double-Click to Edit (`src/lumen/ui/`)

Integrate HitTester into PlotCanvas:

- On double-click: if HitTester finds a series within tolerance,
  open LinePropertyDialog pre-filled with that series's current
  style.
- On dialog OK: update the LineSeries's PlotStyle and name in
  PlotScene, repaint.
- If no series hit: keep current behavior (reset to auto-range).

Disambiguate: double-click on a line = edit properties.
Double-click on empty space = reset view.

### D4 — Series Visibility Toggle

- LineSeries gets a `bool visible_` member (default true).
- PlotRenderer skips invisible series.
- LinePropertyDialog has a Visible checkbox.
- PlotCanvasDock column picker shows visibility state (e.g.,
  strikethrough text or greyed combo for hidden series).

### D5 — Persistence of Custom Styles

When the user changes a series style, it survives:
- Column picker changes (style is preserved per column name)
- View reset (double-click on empty space)

Style does NOT survive:
- Closing and re-opening the file (acceptable for Phase 3a;
  session persistence deferred to Phase 4)

Implementation: PlotCanvasDock maintains a
`QHash<QString, PlotStyle>` mapping column name to custom style.
When rebuilding the scene after column change, apply stored styles.

### D6 — Tests

- Unit tests for HitTester: point on line returns correct series
  index, point far from line returns nullopt, multiple series
  disambiguation.
- Unit tests for LineSeries visibility: visible=false excluded
  from rendering, dataRange still includes hidden series.
- Unit test for LinePropertyDialog: construct, set values, verify
  result (no GUI interaction needed — test the data flow).

## Non-goals (deferred)

- Full property inspector panel (Phase 5)
- Axis property editing (Phase 5)
- Plot title editing via double-click (Phase 5)
- Style persistence across sessions / save to file (Phase 4)
- Right-click context menu on series (Phase 4)
- Scatter/bar/histogram (Phase 3b)
- Data decimation (Phase 3b)
- Dark theme (Phase 3b)

## Acceptance criteria

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
    -DLUMEN_ENABLE_ASAN=ON -DLUMEN_ENABLE_UBSAN=ON
cmake --build build
cd build && ctest --output-on-failure

./build/bin/lumen
# 1. Open electrophysiology CSV
# 2. Auto-plot shows voltage trace
# 3. Double-click ON a line → LinePropertyDialog opens
# 4. Change color to red, width to 3.0 → OK → line updates
# 5. Double-click on EMPTY SPACE → view resets (no dialog)
# 6. Add second Y series, double-click it → edit its style
# 7. Uncheck Visible → series disappears from plot
# 8. Change column picker → custom styles preserved
# 9. All previous interactions still work (pan, zoom, etc.)
```

## Risks

- **Hit-testing performance**: iterating all polyline segments is
  O(n) per series. For 3,500 points x 2 series = 7,000 segments,
  this is <1ms. Acceptable for Phase 3a.
- **Double-click ambiguity**: resolved by checking HitTester first.
  If hit → edit dialog. If no hit → reset view.
- **PlotStyle mutability**: LineSeries currently holds PlotStyle by
  value. Need to make it mutable (setter) or rebuild the series.
  Rebuilding is simpler and avoids breaking const correctness.

## Exit checklist

- [ ] All deliverables done (D1-D6)
- [ ] HitTester tests pass
- [ ] LinePropertyDialog works with real data
- [ ] Double-click disambiguation works
- [ ] Series visibility toggle works
- [ ] Custom styles survive column picker changes
- [ ] All previous interactions unbroken
- [ ] Human verified with real electrophysiology CSV
