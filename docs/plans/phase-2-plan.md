# Phase 2 Plan — Plot Engine (Line Plot)

> Reference: `docs/specs/phase-2-spec.md`

## Task Dependency Graph

```
  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐
  │T1 NiceNumbers│   │T2 ViewTrans- │   │T3 LineSeries │
  │ + CoordMapper│   │   form       │   │ + PlotStyle  │
  │  (backend)   │   │  (backend)   │   │  (backend)   │
  └──────┬───────┘   └──────┬───────┘   └──────┬───────┘
         │                  │                   │
         └──────────┬───────┘───────────────────┘
                    ▼
           ┌────────────────┐
           │T4 Axis + Plot- │
           │   Scene layout │
           │   (backend)    │
           └────────┬───────┘
                    │
         ┌──────────┼──────────────┐
         ▼          ▼              ▼
  ┌────────────┐ ┌──────────┐ ┌──────────────┐
  │T5 Plot-    │ │T8 QA     │ │T9 Plot       │
  │ Renderer   │ │ tests    │ │ module CMake │
  │ (frontend) │ │  (qa)    │ │ (backend)    │
  └─────┬──────┘ └────┬─────┘ └──────┬───────┘
        │              │              │
        ▼              │              │
  ┌────────────────┐   │              │
  │T6 PlotCanvas + │   │              │
  │ PlotCanvasDock │   │              │
  │ + Interaction  │   │              │
  │  (frontend)    │   │              │
  └────────┬───────┘   │              │
           │           │              │
           └─────┬─────┘──────────────┘
                 ▼
        ┌─────────────────┐
        │T7 Plot creation │
        │ flow + column   │
        │ picker (front.) │
        └────────┬────────┘
                 ▼
        ┌─────────────────┐
        │T10 Docs update  │
        │   (docs)        │
        └─────────────────┘
```

## Critical path

T1 + T2 + T3 (parallel) → T4 → T5 → T6 → T7

Estimated: ~10–14 working days on the critical path.
T8, T9 run in parallel.

---

## Tasks

### T1 — NiceNumbers + CoordinateMapper (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/NiceNumbers.h` / `.cpp`
- `src/lumen/plot/CoordinateMapper.h` / `.cpp`
- `tests/unit/test_nice_numbers.cpp`
- `tests/unit/test_coordinate_mapper.cpp`

**Subtasks**:
- T1.1: `NiceNumbers` — Given data range (min, max) and target
  tick count (~5–10), produce a list of "nice" tick values using
  the 1-2-5 pattern. Handle: fractional ranges (e.g., -38.7 to
  -37.9 mV), zero-crossing ranges, very small ranges (< 0.01),
  very large ranges (> 1e6). Return tick values + tick label format.
- T1.2: `CoordinateMapper` — Bidirectional mapping. Constructor
  takes data range (xMin, xMax, yMin, yMax) and pixel rect
  (QRectF). `dataToPixel(double x, double y) → QPointF` and
  `pixelToData(QPointF) → (double, double)`. Y axis inverted
  (data Y up, pixel Y down).
- T1.3: Unit tests for both: round-trip accuracy, edge cases
  (zero range, negative range, single-point range).

**Acceptance criteria**:
- NiceNumbers produces human-readable ticks for the reference
  data ranges: time_ms [0, 700] and voltage_mV [-80, 40]
- CoordinateMapper round-trips within 1e-10 precision
- Tests pass under ASan+UBSan

**Dependencies**: none (can start immediately)

---

### T2 — ViewTransform (Backend, Size: S)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/ViewTransform.h` / `.cpp`
- `tests/unit/test_view_transform.cpp`

**Subtasks**:
- T2.1: `ViewTransform` — Stores current pan offset (dx, dy in
  data units) and zoom factor (scaleX, scaleY). Methods:
  `pan(dx, dy)`, `zoom(factor, centerDataX, centerDataY)`,
  `zoomX(factor, center)`, `zoomY(factor, center)`,
  `reset()`. Applies to a base data range to produce a view range.
- T2.2: Unit tests: pan, zoom, zoom-at-point, reset, X/Y
  independent zoom.

**Acceptance criteria**:
- Zoom centered on a point keeps that point stationary
- Reset returns to original data range
- Tests pass under ASan+UBSan

**Dependencies**: none (can start immediately, parallel with T1)

---

### T3 — LineSeries + PlotStyle (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/LineSeries.h` / `.cpp`
- `src/lumen/plot/PlotStyle.h`
- `tests/unit/test_line_series.cpp`

**Subtasks**:
- T3.1: `PlotStyle` — Struct: QColor color, double lineWidth,
  Qt::PenStyle lineStyle (Solid, Dash, Dot, DashDot). Static
  method `fromPalette(int index)` assigns color from
  `tokens::color::plotPalette`.
- T3.2: `LineSeries` — Holds const Column* xCol, const Column*
  yCol, PlotStyle style, QString name. Method
  `buildPolylines() → std::vector<QPolygonF>`: produces one
  QPolygonF per contiguous non-NaN segment (breaking at NaN).
  Method `dataRange() → (xMin, xMax, yMin, yMax)` ignoring NaN.
- T3.3: Unit tests: NaN gap splitting (1 gap, multiple gaps,
  all NaN, no NaN), data range with NaN, style palette assignment.

**Acceptance criteria**:
- Reference data dvdt_3pt_bwd column (has NaN at rows 0–1)
  produces 2+ polylines with correct gap
- Data range excludes NaN values
- Tests pass under ASan+UBSan

**Dependencies**: none (can start immediately, parallel with T1, T2)

---

### T4 — Axis + PlotScene Layout (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/plot/Axis.h` / `.cpp`
- `src/lumen/plot/PlotScene.h` / `.cpp`
- `tests/unit/test_axis.cpp`
- `tests/unit/test_plot_scene.cpp`

**Subtasks**:
- T4.1: `Axis` — Orientation (Horizontal, Vertical). Data range,
  label (QString). Uses NiceNumbers for ticks. Methods:
  `setRange(min, max)`, `autoRange(LineSeries[])`,
  `ticks() → vector<TickMark>` where TickMark = {value, label}.
- T4.2: `PlotScene` — Owns xAxis, yAxis, vector<LineSeries>,
  title (QString). Given a pixel size (QSizeF), computes layout:
  margins for axis labels, tick labels, title, legend. Returns
  `plotArea() → QRectF` (the data drawing region).
  `addSeries(LineSeries)`, `removeSeries(int)`, `autoRange()`.
- T4.3: Unit tests: axis auto-range from series, layout margins
  with/without title, scene with 0/1/multiple series.

**Acceptance criteria**:
- PlotScene with reference data series computes a reasonable
  plotArea (margins ~60px left, ~40px bottom, ~20px top/right)
- Axis ticks for voltage range produce readable labels
- Tests pass under ASan+UBSan

**Dependencies**: T1 (NiceNumbers), T2 (ViewTransform), T3 (LineSeries)

---

### T5 — PlotRenderer (Frontend, Size: L)

**Owner**: frontend

**Files to create**:
- `src/lumen/plot/PlotRenderer.h` / `.cpp`
- `tests/unit/test_plot_renderer.cpp`

**Subtasks**:
- T5.1: `PlotRenderer::render(QPainter&, PlotScene&, QSizeF)` — 
  Main entry. Calls sub-methods in order:
  1. Fill background (background.primary)
  2. Compute layout via PlotScene
  3. Draw grid lines (border.subtle, 1px, dashed)
  4. Draw axis lines (border.strong, 1px)
  5. Draw tick marks (4px outside plot area)
  6. Draw tick labels (text.secondary, footnote)
  7. Draw axis labels (text.primary, body-strong)
  8. Set clip region to plot area
  9. Draw each LineSeries (anti-aliased, per-series style)
  10. Draw title (text.primary, title-3, centered above plot)
  11. Draw legend (text.secondary, body, top-right or bottom)
- T5.2: All colors and sizes from DesignTokens (no literals).
- T5.3: Unit test: render to QImage, verify image is non-empty,
  verify pixel at known location has expected color (e.g., 
  background color outside plot area).

**Acceptance criteria**:
- Renders a complete plot to a QImage without crash
- Uses DesignTokens colors exclusively
- Anti-aliased line rendering
- Clip region prevents lines from overflowing into axis area
- Tests pass under ASan+UBSan

**Dependencies**: T4 (PlotScene, Axis)

---

### T6 — PlotCanvas + PlotCanvasDock + Interaction (Frontend, Size: L)

**Owner**: frontend

**Files to create**:
- `src/lumen/ui/PlotCanvas.h` / `.cpp`
- `src/lumen/ui/PlotCanvasDock.h` / `.cpp`
- `tests/unit/test_plot_canvas.cpp` (basic: widget creates, accepts
  a PlotScene, can render)

**Subtasks**:
- T6.1: `PlotCanvas` — QWidget. Owns PlotScene pointer.
  `paintEvent` calls PlotRenderer. `setPlotScene(PlotScene*)`.
  Minimum size 200×150.
- T6.2: Mouse interaction:
  - Left drag: pan (modify ViewTransform, update, repaint)
  - Scroll wheel: zoom centered on cursor (uniform)
  - Shift+scroll: X-only zoom
  - Ctrl+scroll: Y-only zoom
  - Right drag: zoom box (draw rubber band, zoom on release)
  - Double-click: reset to auto-range
- T6.3: Crosshair overlay: on mouse move, draw thin crosshair
  lines + tooltip with data coordinates (formatted to 4 sig digits).
  Use `CoordinateMapper::pixelToData`.
- T6.4: `PlotCanvasDock` — QDockWidget wrapping PlotCanvas.
  Added to MainWindow at Qt::RightDockWidgetArea. View menu toggle.
- T6.5: Basic unit test: create PlotCanvas, set scene, render
  to QPixmap, verify non-null.

**Acceptance criteria**:
- Pan, zoom (uniform + axis-specific), zoom-box, reset all work
- Crosshair shows correct data coordinates
- Dock integrates with MainWindow
- No ASan/UBSan errors during interaction

**Dependencies**: T5 (PlotRenderer)

---

### T7 — Plot Creation Flow + Column Picker (Frontend, Size: M)

**Owner**: frontend

**Files to modify**:
- `src/lumen/ui/MainWindow.h` / `.cpp` (add PlotCanvasDock, 
  auto-plot on file load)
- `src/lumen/ui/PlotCanvasDock.h` / `.cpp` (add column picker
  combo boxes)

**Subtasks**:
- T7.1: On CSV open success (FileLoader::finished), auto-create
  a PlotScene with first numeric column as X, second as Y. Show
  PlotCanvasDock.
- T7.2: Column picker: two QComboBox in a QToolBar at the top of
  PlotCanvasDock. "X Column" and "Y Column", populated from
  DataFrame numeric column names. Changing selection rebuilds the
  LineSeries and repaints.
- T7.3: "Add Series" button: adds another Y combo box + removes
  button. New series gets next palette color. "Remove" button per
  extra series.
- T7.4: Plot title auto-set to "Y vs X" (e.g., "voltage_mV vs
  time_ms").

**Acceptance criteria**:
- Opening CSV auto-shows a plot
- Changing columns updates plot immediately
- Multiple Y series shown with different colors
- Auto-title reflects column names

**Dependencies**: T6 (PlotCanvas/Dock)

---

### T8 — QA Plot Tests (QA, Size: M) ⚡ parallel

**Owner**: qa

**Files to create**:
- `tests/unit/test_nice_numbers.cpp` (if not done by backend)
- `tests/unit/test_view_transform.cpp` (if not done by backend)
- `tests/integration/test_plot_creation.cpp`

**Subtasks**:
- T8.1: Review and extend backend unit tests if gaps found.
- T8.2: Integration test: load electrophys CSV → auto-create
  plot → verify PlotScene has correct axis ranges and series count.
- T8.3: NaN gap test: load CSV with NaN column, verify
  LineSeries::buildPolylines() produces multiple segments.

**Acceptance criteria**:
- All plot-related tests pass
- Integration test covers the full flow
- ASan/UBSan clean

**Dependencies**: T4 (PlotScene exists to test against)

---

### T9 — Plot Module CMake (Backend, Size: S) ⚡ parallel

**Owner**: backend

**Files to create**:
- `src/lumen/plot/CMakeLists.txt` (static library `lumen_plot`)

**Files to modify**:
- `src/lumen/CMakeLists.txt` (add_subdirectory(plot), link lumen_plot)
- `tests/unit/CMakeLists.txt` (link lumen_plot)

**Subtasks**:
- T9.1: `lumen_plot` links Qt6::Core, Qt6::Gui, lumen_data,
  lumen_style. Does NOT link Qt6::Widgets (plot/ is UI-independent).
- T9.2: Main lumen target links lumen_plot.
- T9.3: Test target links lumen_plot.

**Acceptance criteria**:
- Build succeeds with plot module
- plot/ does not depend on ui/

**Dependencies**: none (create early, T1/T2/T3 add files to it)

---

### T10 — Docs Update (Docs, Size: S)

**Owner**: docs

**Files to modify**:
- `README.md` (update with plot features)
- `src/lumen/plot/CLAUDE.md` (update with actual responsibilities)
- `src/lumen/ui/CLAUDE.md` (add PlotCanvas, PlotCanvasDock)
- `CLAUDE.md` (update current phase)

**Acceptance criteria**:
- README mentions plot capability
- Per-module CLAUDE.md matches actual code

**Dependencies**: T7 (after features merged)

---

## Parallel Execution Schedule

```
Week 1:
  Backend:  T9 (CMake) → T1 (NiceNumbers+CoordMapper) ──────►
            T2 (ViewTransform) ──►
            T3 (LineSeries+PlotStyle) ──────►
  QA:       T8 start (review tests as backend delivers)

Week 2:
  Backend:  T4 (Axis+PlotScene) ──────────────────────►
  Frontend: T5 (PlotRenderer) ─────────────────────────►
  QA:       T8 continue (extend tests)

Week 3:
  Frontend: T6 (PlotCanvas+Interaction) ────────────────►
  QA:       T8 integration tests

Week 4:
  Frontend: T7 (Plot creation flow + column picker) ───►
  Docs:     T10
  Integration: merge PRs
```

## Phase 1 lessons applied

1. **Fixture ownership**: QA exclusively owns test fixtures.
   Backend uses inline test data in unit tests.
2. **CMake module early**: T9 creates lumen_plot CMake target
   before implementation starts, avoiding build integration pain.
3. **shared_ptr convention**: LineSeries holds raw const Column*
   (non-owning). DataFrame lifetime managed by DocumentRegistry
   (shared_ptr). No new ownership patterns needed.

## Agent Launch Order

1. **Now**: Architect plan → human approval → merge to main.
2. **After plan merged**: Backend (T9+T1+T2+T3) + QA (T8 start).
3. **When T1-T3 done**: Backend (T4) + Frontend (T5).
4. **When T4-T5 done**: Frontend (T6+T7) + QA (T8 integration).
5. **After merge**: Docs (T10).
