# Phase 2 — Plot Engine (Line Plot)

## Goal

Render a 2D line plot from DataFrame columns using a self-built
engine on QPainter. The user selects X and Y columns, sees an
interactive plot with pan, zoom, crosshair, and axis ticks. This is
the core of the MATLAB-figure experience.

Phase 2 delivers **line plots only**. Scatter, bar, histogram, and
other plot types are deferred to Phase 3+.

## Active agents

- Architect (spec, plan, ADRs)
- Backend (plot data pipeline, coordinate transforms)
- Frontend (PlotCanvas widget, interaction, axis rendering)
- QA (plot engine tests, visual regression fixtures)
- Integration (merge PRs)
- Docs (update README, CLAUDE.md)

## Dependencies on Phase 1

Phase 2 builds on these Phase 1 APIs:

- `lumen::data::DataFrame` — column access, row/col counts
- `lumen::data::Column` — typed data vectors (doubleData(), name())
- `lumen::data::ColumnType` — Int64, Double, String
- `lumen::core::DocumentRegistry` — get open DataFrames
- `lumen::core::EventBus` — cross-module notifications
- `lumen::tokens::color::plotPalette` — 8-color categorical palette
- `lumen::tokens::plot::*` — line width, marker size, axis/grid widths

## Reference data

Same electrophysiology CSV as Phase 1:
- X axis: `time_ms` (0.2 to 699.8, step 0.2)
- Y axis: `voltage_mV` (range ~-80 to +40 mV, action potentials)
- Also: `I_ion_nA`, `dvdt_*` columns for multi-series
- Contains NaN — plot engine must skip NaN points (gap in line)

Expected plot: a classic voltage trace with depolarization spikes,
repolarization, and a resting potential baseline around -38 mV.

## Deliverables

### D1 — Plot Object Model (`src/lumen/plot/`)

The object model defines what the user sees and can (later)
interact with.

- **`PlotScene`** — Top-level container. Owns axes, series, title,
  legend. Knows its pixel size. Computes layout (margins for axes,
  title, legend).
- **`Axis`** — Horizontal or vertical. Data range (min, max),
  auto-range from data, tick generation (nice numbers algorithm),
  tick labels, axis label. Grid lines.
- **`LineSeries`** — References a pair of Column pointers (X, Y)
  from a DataFrame. Stores visual properties: color, line width,
  line style (solid, dashed, dotted). Handles NaN gaps.
- **`PlotStyle`** — Per-series visual properties. Uses
  DesignTokens plot palette for automatic color assignment.

### D2 — Coordinate System (`src/lumen/plot/`)

- **`CoordinateMapper`** — Bidirectional mapping between data
  coordinates (double, double) and pixel coordinates (QPointF).
  Handles axis orientation (Y up in data, Y down in pixels).
- **`ViewTransform`** — Current pan offset + zoom level. Modified
  by interaction. Used by CoordinateMapper.
- **`NiceNumbers`** — Tick spacing algorithm. Given a data range
  and available pixel length, produce "nice" tick values
  (1, 2, 5 × 10^n pattern).

### D3 — Rendering Engine (`src/lumen/plot/`)

All rendering through QPainter. No OpenGL in Phase 2.

- **`PlotRenderer`** — Takes a PlotScene and a QPainter, renders
  everything:
  - Background (background.primary)
  - Grid lines (border.subtle, dashed)
  - Axis lines (border.strong)
  - Tick marks and tick labels (text.secondary, footnote size)
  - Axis labels (text.primary, body-strong)
  - Line series (anti-aliased, configurable width/color/dash)
  - Plot title (text.primary, title-3)
  - Legend (text.secondary, body)
- **Clip region**: Data area is clipped so lines don't draw over axes.
- **Anti-aliasing**: QPainter::Antialiasing + TextAntialiasing.
- **NaN handling**: Break the line at NaN values, resume after.

### D4 — PlotCanvas Widget (`src/lumen/ui/`)

- **`PlotCanvas`** — QWidget subclass. Owns a `PlotScene`. Overrides
  `paintEvent` to call PlotRenderer. Handles mouse events for
  interaction.
- **`PlotCanvasDock`** — QDockWidget wrapping PlotCanvas. Added to
  MainWindow, toggled from View menu.

### D5 — Interaction (`src/lumen/plot/` or `src/lumen/ui/`)

Mouse-driven interaction on the PlotCanvas:

- **Pan**: Click and drag (left button) moves the view.
- **Zoom**: Scroll wheel zooms in/out centered on cursor position.
  Uniform zoom (both axes). Shift+scroll for X-only, Ctrl+scroll
  for Y-only.
- **Zoom box**: Right-click drag draws a rectangle, releases to
  zoom to that region.
- **Reset**: Double-click resets to auto-range (fit all data).
- **Crosshair**: Mouse hover shows crosshair lines + coordinate
  tooltip at cursor position (data coordinates, formatted).

### D6 — Plot Creation Flow

When the user opens a CSV (Phase 1 flow), Lumen should:

1. Show data in DataTableDock (Phase 1, already works).
2. Auto-create a plot: first numeric column as X, second as Y.
3. Show PlotCanvasDock with the default plot.
4. User can change X/Y column assignments via a simple column
   picker (combo boxes in a toolbar or dock header).

Column picker UI:
- Two combo boxes: "X Column" and "Y Column"
- Populated from DataFrame column names (numeric columns only)
- Changing selection updates the plot immediately
- Allow multiple Y series: "Add Series" button adds another Y
  combo box with next palette color

### D7 — Tests

- Unit tests for CoordinateMapper (data↔pixel round-trip)
- Unit tests for NiceNumbers (known input→output pairs)
- Unit tests for LineSeries NaN gap handling
- Unit tests for ViewTransform (pan, zoom, reset)
- Unit test for PlotScene layout computation
- Integration test: load CSV → create plot → verify scene contains
  correct data range and series count

## Non-goals (deferred)

- Scatter plot, bar chart, histogram (Phase 3)
- Logarithmic axes (Phase 3)
- Multiple Y axes (Phase 4)
- Property inspector (Phase 5) — double-click editing
- Export to PNG/SVG (Phase 4)
- Data decimation / LOD for large datasets (Phase 3)
- Dark theme for plots (Phase 3, with dark QSS)
- Annotations, text labels on plot (Phase 4)
- Real-time streaming data (Phase 6)

## Architecture fit

```
┌──────────────────────────────────────────┐
│  PlotCanvasDock (ui/)                    │
│    └── PlotCanvas (QWidget)              │
│          ├── paintEvent → PlotRenderer   │
│          └── mouse events → Interaction  │
└──────────────┬───────────────────────────┘
               │ owns
               ▼
┌──────────────────────────────────────────┐
│  PlotScene (plot/)                       │
│    ├── Axis (x, y)                       │
│    ├── LineSeries[] → Column* (data/)    │
│    ├── PlotStyle                         │
│    ├── ViewTransform                     │
│    └── CoordinateMapper                  │
└──────────────────────────────────────────┘
               │ reads
               ▼
┌──────────────────────────────────────────┐
│  DataFrame / Column (data/)              │
│  DocumentRegistry (core/)                │
└──────────────────────────────────────────┘
```

Layering rules (from architecture.md):
- `plot/` may import from `core/` and `style/` (for tokens)
- `plot/` may import from `data/` (for Column/DataFrame read access)
- `ui/` may import from `plot/`, `core/`, `style/`, `data/`
- `plot/` does NOT import from `ui/`

## Performance targets

- Render 10,000-point line plot in < 16 ms (60 fps)
- Pan/zoom interaction feels instant (no perceptible lag)
- Startup: auto-plot of reference CSV (3,500 points) appears
  within 200 ms of file load completing

## Acceptance criteria

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
    -DLUMEN_ENABLE_ASAN=ON -DLUMEN_ENABLE_UBSAN=ON
cmake --build build
cd build && ctest --output-on-failure

./build/bin/lumen
# 1. File → Open CSV → select electrophysiology CSV
# 2. Line plot appears automatically (time_ms vs voltage_mV)
# 3. Pan by dragging
# 4. Zoom with scroll wheel
# 5. Crosshair shows coordinates on hover
# 6. Double-click resets zoom to auto-range
# 7. Change Y column via combo box → plot updates
# 8. Add second Y series → two lines with different colors
# 9. NaN gaps visible in derivative columns
# 10. All axes have nice tick marks and labels
# 11. No ASan/UBSan errors throughout
```

## Risks

- **QPainter performance with many points**: 3,500 points is fine.
  100K+ points may need decimation (deferred to Phase 3). For now,
  plot all points.
- **HiDPI rendering**: QPainter handles device pixel ratio
  automatically in Qt 6. Test on a HiDPI display if available.
- **Coordinate precision**: Electrophysiology data has very small
  Y values (~-38.5 mV). Tick labels must show enough decimal places.
  NiceNumbers algorithm must handle fractional ranges well.
- **NaN in X column**: If X column has NaN, skip the entire point.
  Not just Y-NaN gaps.

## Exit checklist

- [ ] All deliverables done (D1–D7)
- [ ] All plot unit tests pass
- [ ] Owner can see their voltage trace plotted
- [ ] Pan, zoom, crosshair all work
- [ ] Multi-series with column picker works
- [ ] CI green on main
- [ ] `docs/reviews/phase-2-review.md` written
- [ ] `docs/specs/phase-3-spec.md` drafted
