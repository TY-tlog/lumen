# Lumen Architecture (C++/Qt)

## High-level view

```
┌──────────────────────────────────────────────────────────────┐
│  UI Layer  (QMainWindow + dock widgets, all of src/lumen/ui) │
│  ├── MainWindow                                               │
│  ├── FileExplorerDock        (Phase 1+)                       │
│  ├── PlotCanvasDock          (Phase 2+)                       │
│  └── PropertyInspectorDock   (Phase 5+)                       │
└──────────────────────────────────────────────────────────────┘
                            ▲
                            │ Qt signals/slots, CommandBus
                            ▼
┌──────────────────────────────────────────────────────────────┐
│  Core   (src/lumen/core)                                      │
│  ├── EventBus            – decoupled cross-module signals     │
│  ├── CommandBus          – undo/redo                          │
│  ├── DocumentRegistry    – open files                         │
│  ├── PlotRegistry        – plot handles                       │
│  └── SelectionStore      – global selection                   │
└──────────────────────────────────────────────────────────────┘
       ▲                ▲                  ▲
       │                │                  │
┌──────┴─────┐   ┌──────┴─────┐    ┌───────┴──────┐
│   Data     │   │   Plot     │    │   Style      │
│ (CSV       │   │ (QPainter  │    │  (Design     │
│  parser,   │   │  engine,   │    │   system,    │
│  model)    │   │  objects,  │    │   QSS gen)   │
│            │   │  hit test) │    │              │
└────────────┘   └────────────┘    └──────────────┘
```

## Threading model

- Main thread: Qt event loop, all widget operations
- Worker threads (`QThreadPool` or `QtConcurrent`): file parsing,
  any computation > 50 ms
- Result delivery: queued signals back to main thread

## Plot engine pipeline (Phase 2 sketch)

```
Document → DataSeries → PlotItem (Line/Scatter/...)
                            │
                            ▼
                       SceneGraph
                            │
                            ▼
                       PaintCommands
                            │
                            ▼
                        QPainter
```

## Event flow

User input → Widget → Command construction → CommandBus.execute
  → state mutation → EventBus.emit → subscribers (UI, plot, …) repaint

## Layering rules

1. UI may import from core, plot, style.
2. Plot may import from core, style.
3. Data may import from core only.
4. Core imports nothing from above layers.
5. Util is leaf; nothing depends on it that depends back.

Enforced by code review and (later) custom CMake checks.

## Phase 1 additions

### Data-loading flow

```
User clicks File → Open CSV
        │
        ▼
  QFileDialog (main thread)
        │
        ▼
  FileLoader::load(path)
        │  creates QThread, moves worker to it
        ▼
  ┌─────────────────────────────┐
  │  Worker thread              │
  │  CsvReader::parse(path)     │
  │    → tokenize rows          │
  │    → infer column types     │
  │    → build DataFrame        │
  │  signal: progress(percent)  │
  │  signal: finished(DataFrame)│
  └─────────────────────────────┘
        │  queued connection
        ▼
  DocumentRegistry::addDocument(path, DataFrame)
        │
        ▼
  EventBus::emit(DocumentOpened, path)
        │
        ▼
  DataTableDock receives event → sets DataFrameTableModel
```

### Threading model (ADR-009)

- **Main thread**: Qt event loop, all widget operations, EventBus
  dispatch, DocumentRegistry access.
- **File-loading thread**: one `QThread` per load. Runs CsvReader
  synchronously. Delivers result via queued signal. Thread is
  destroyed after completion.
- **Cancellation**: `std::atomic<bool>` flag, checked every ~1000
  rows during parsing.
- **Thread safety**: `DataFrame` is move-only. Moved from worker
  thread to main thread in the `finished` signal. No shared
  mutable state.

### EventBus (ADR-010)

Two-tier communication:
1. **Direct Qt signals**: local, within a module (widget → widget).
2. **EventBus**: cross-module, decoupled (document opened, selection
   changed, theme changed).
3. **CommandBus** (Phase 3+): state-changing operations with undo.

### Data model

```
DataFrame
  ├── Column "time_ms"        (Double)
  ├── Column "voltage_mV"     (Double)
  ├── Column "I_ion_nA"       (Double, contains NaN)
  └── ...
```

- `ColumnType` enum: `Int64`, `Double`, `String`
- NaN stored as `std::numeric_limits<double>::quiet_NaN()`
- Move-only, no copy

## Phase 2 additions — Plot engine

### Rendering pipeline (as implemented)

```
DataFrame (data/)
    │
    ▼
LineSeries (plot/)
    │  holds const Column* xCol, yCol
    │  buildPolylines() → vector<QPolygonF> (breaks at NaN)
    │  dataRange() → {xMin, xMax, yMin, yMax}
    ▼
PlotScene (plot/)
    │  owns: Axis xAxis, Axis yAxis, vector<LineSeries>, ViewTransform
    │  autoRange() → sets axes + ViewTransform base range
    │  computePlotArea(widgetSize) → QRectF (hardcoded margins, ADR-013)
    ▼
PlotRenderer (plot/)
    │  render(QPainter&, PlotScene&, QSizeF)
    │  draws: background, grid, axes, tick marks/labels, axis labels,
    │         clipped line series, title, legend
    │  all colors from DesignTokens (no literals)
    ▼
PlotCanvas (ui/)
    │  QWidget, owns PlotScene*
    │  paintEvent() → PlotRenderer::render()
    │  mouse events → interaction (inline, ADR-016)
    ▼
PlotCanvasDock (ui/)
       QDockWidget wrapping PlotCanvas + column picker toolbar
       setDataFrame() → populates combo boxes, rebuilds PlotScene
       auto-shown on CSV open
```

### Coordinate system

- `CoordinateMapper` — bidirectional linear mapping, data ↔ pixel,
  Y inverted. Double precision, 1e-10 round-trip (ADR-014).
- `ViewTransform` — base range + current view range. Pan, zoom
  (uniform, X-only, Y-only), reset to base.
- `NiceNumbers` — 1-2-5 Heckbert algorithm for tick generation
  (ADR-015). Handles arbitrary ranges including fractional,
  negative, and very large/small.

### Interaction model (ADR-016)

All interaction lives inline in PlotCanvas mouse event handlers:

| Input | Action |
|-------|--------|
| Left drag | Pan (ViewTransform::pan) |
| Scroll wheel | Zoom centered on cursor (ViewTransform::zoom) |
| Shift + scroll | X-only zoom (ViewTransform::zoomX) |
| Ctrl + scroll | Y-only zoom (ViewTransform::zoomY) |
| Right drag | Zoom box (set ViewTransform range to box) |
| Double-click | Reset to auto-range (PlotScene::autoRange) |
| Mouse hover | Crosshair + data coordinate tooltip (ADR-017) |

### Known tech debt (Phase 2)

1. **Hardcoded margins** in PlotScene::computePlotArea() — 60/50/30/15
   pixels (ADR-013). Refactor to QFontMetrics in Phase 4.
2. **Inline interaction** in PlotCanvas — ~150 lines, manageable now
   but extract to InteractionController when adding new modes
   (ADR-016).
3. **Crosshair shows cursor position**, not nearest data point
   (ADR-017). Upgrade to nearest-point snap with HitTester in Phase 4.
4. **No PlotRegistry** (Phase 2.5 adds it). Tracks which PlotCanvas
   belongs to which document for cross-module event propagation.

## Phase 3a additions — Line property editing

### CommandBus (ADR-018)

```
User double-clicks line series
        │
        ▼
InteractionController::handleDoubleClick()
        │  calls HitTester::hitTest()
        ▼
HitTester returns seriesIndex
        │
        ▼
InteractionController emits seriesDoubleClicked(index)
        │
        ▼
PlotCanvasDock opens LinePropertyDialog
        │  pre-filled with current PlotStyle + name + visibility
        ▼
User edits properties → OK
        │
        ▼
PlotCanvasDock creates ChangeLineStyleCommand
        │  captures: old style, new style, PlotScene*, index
        ▼
CommandBus::execute(command)
        │  calls command->execute() → LineSeries::setStyle()
        │  pushes to undo stack
        ▼
PlotCanvas repaints with new style
```

Undo: `CommandBus::undo()` pops command, calls `command->undo()`
which restores old PlotStyle via LineSeries setters.

CommandBus lives in `src/lumen/core/`, owned by Application.
Command is an abstract base with `execute()`, `undo()`,
`description()`. Concrete commands in `src/lumen/core/commands/`.

### HitTester (ADR-019)

Lives in `src/lumen/plot/` (UI-independent). Static method:
`HitTester::hitTest(PlotScene, CoordinateMapper, pixelPos, tolerance)`
→ `optional<HitResult{seriesIndex, pixelDistance}>`.

Algorithm: brute-force point-to-segment distance for each visible
series's polylines mapped to pixel space. O(n) per series. <1ms
for reference data (7,000 segments).

Resolves ADR-017 foundation. Phase 4 extends with `hitTestPoint()`
for nearest-point crosshair (binary search on sorted X column).

### InteractionController (ADR-020)

Lives in `src/lumen/ui/`. Extracted from PlotCanvas (resolves
ADR-016). Owns all interaction state and mouse event logic.

PlotCanvas becomes a thin rendering host (~50 lines):
- `paintEvent()` → PlotRenderer + overlays from controller state
- Mouse events → forward to `controller_->handleXxx()`
- Connects controller signals for double-click outcomes

InteractionController modes: Idle, Panning, ZoomBoxing.
Double-click: HitTester query → `seriesDoubleClicked(index)` or
`emptyAreaDoubleClicked()`.

### LineSeries mutability

LineSeries gains setters: `setStyle()`, `setName()`, `setVisible()`.
PlotScene gains mutable access: `seriesAt(index) → LineSeries&`.
PlotRenderer skips invisible series. `dataRange()` still includes
invisible series (so auto-range doesn't shift when hiding a series).

### Style persistence

PlotCanvasDock maintains `QHash<QString, PlotStyle> customStyles_`
and `QHash<QString, bool> customVisibility_` mapping column names
to user-customized properties. When `rebuildPlot()` recreates
LineSeries after column picker changes, it applies stored custom
styles. Styles persist within a session but not across file
close/reopen (session persistence deferred to Phase 4).

### Updated layering rules

1. UI may import from core, plot, style, data.
2. Plot may import from core, style, data.
3. Core (including CommandBus) imports nothing from above layers.
   Commands reference PlotScene and LineSeries from plot/ — this
   is acceptable because commands are created by UI code and
   passed to CommandBus as `unique_ptr<Command>`. CommandBus itself
   only calls the `Command` interface methods.
4. Data imports from core only.
5. Util is leaf.

### Resolved tech debt

| Phase 2 debt | Resolution | ADR |
|-------------|------------|-----|
| Inline interaction in PlotCanvas | Extracted to InteractionController | ADR-016 → ADR-020 |
| No hit-testing (cursor crosshair only) | HitTester in plot/ | ADR-017 → ADR-019 |
| No undo/redo | CommandBus + ChangeLineStyleCommand | ADR-006 → ADR-018 |

### Remaining tech debt (after Phase 3a)

1. **Hardcoded margins** (ADR-013) — to be resolved in Phase 3b (T6).
2. **Crosshair cursor position** (ADR-017) — to be resolved in Phase 3b (T6.5).
3. **No command merging** — rapid edits create many undo entries.
   Phase 5 can add merge logic for commands within a time window.

## Phase 3b additions — Axis, title, legend editing + dynamic margins

### New command groups (bundled pattern from Phase 3a)

Three new command classes in `src/lumen/core/commands/`, each
following the ChangeLineStyleCommand bundled pattern (one command
captures all properties of one element):

- `ChangeAxisPropertiesCommand` — label, range mode, manual
  min/max, tick count, tick format, grid visible
- `ChangeTitleCommand` — title text, font size, font weight
- `ChangeLegendCommand` — position, visible

Commands hold non-owning pointers to Axis/PlotScene/Legend and
capture old+new values. Execute applies new, undo restores old.

### Axis setter additions

Axis becomes a QObject (for changed() signal). New setters:
setLabel, setRangeMode (Auto/Manual), setManualRange, setTickCount,
setTickFormat (Auto/Scientific/Fixed), setGridVisible. Each setter
compares old vs new before emitting changed().

New enums: `RangeMode { Auto, Manual }`,
`TickFormat { Auto, Scientific, Fixed }`.

### PlotScene title state expansion

PlotScene gains setTitleFontPx(int) and
setTitleWeight(QFont::Weight). PlotRenderer reads these when
drawing the title instead of hardcoding title-3 font.

### Legend class extraction

`Legend` class extracted from PlotRenderer's inline legend code.
QObject with Position enum (TopLeft/TopRight/BottomLeft/
BottomRight/OutsideRight), setPosition, setVisible, changed()
signal. PlotScene owns a Legend instance. PlotRenderer reads Legend
state for placement and visibility.

### HitTester extension

Two new methods added without modifying the existing hitTest()
interface:

- `hitNonSeriesElement(scene, mapper, pixelPos)` → `RegionHitResult`
  with HitKind enum (None/XAxis/YAxis/Title/Legend/PlotArea).
  Precedence per ADR-024: LineSeries > Title > Legend > Axis >
  PlotArea.
- `hitTestPoint(scene, mapper, pixelPos, maxDist)` →
  `optional<PointHitResult{seriesIndex, sampleIndex, dataPoint,
  pixelDistance}>`. Binary search on sorted X column for nearest
  actual data sample. **Resolves ADR-017**: crosshair snaps to
  real data points instead of showing interpolated cursor position.

### InteractionController dispatch

New signals following Phase 3a's separate-signal pattern:
xAxisDoubleClicked, yAxisDoubleClicked, titleDoubleClicked,
legendDoubleClicked. Phase 3a's seriesDoubleClicked and
emptyAreaDoubleClicked unchanged.

New mode: EditingTitleInline (suppresses mouse events while inline
title editor is active).

Double-click dispatch:
1. hitTest() for series → seriesDoubleClicked (Phase 3a)
2. hitNonSeriesElement() → dispatch by HitKind
3. PlotArea fallback → emptyAreaDoubleClicked

### PlotScene::computeMargins() — resolves ADR-013

Replaces hardcoded 60/50/30/15 with content-driven computation
(ADR-022):

```
left  = max(Y tick label widths) + spacing::md + Y label height + spacing::sm
bottom = X tick label height + spacing::md + X label height + spacing::sm
top   = title ? titleFm.height() + spacing::md : spacing::sm
right = legend OutsideRight ? legend width + spacing::md : spacing::md
```

1-pixel debounce threshold prevents jiggle during live edits.

### Inline title editor

QLineEdit overlay positioned at the title rect within PlotCanvas.
Double-click title area → editor appears. Enter confirms
(ChangeTitleCommand), Esc cancels, focus-lost applies.
InteractionController enters EditingTitleInline mode.

### Three new dialogs (non-modal, ADR-021)

- AxisDialog: label, range mode, manual min/max, tick count,
  tick format, grid visible. Opens on double-click axis.
- TitleDialog: font size, weight. Opens on right-click title.
- LegendDialog: position, visible, series name table. Opens on
  double-click legend.

All follow LinePropertyDialog's API pattern from Phase 3a.

### Resolved tech debt (cumulative)

| Phase debt | Resolution | Phase | ADR |
|-----------|------------|-------|-----|
| Inline interaction | InteractionController extracted | 3a | ADR-016 → ADR-020 |
| No hit-testing | HitTester in plot/ | 3a | ADR-017 → ADR-019 |
| No undo/redo | CommandBus + commands | 3a | ADR-006 → ADR-018 |
| Hardcoded margins | computeMargins() | 3b | ADR-013 → ADR-022 |
| Cursor crosshair | hitTestPoint() nearest-sample | 3b | ADR-017 → T6.5 |

### Remaining tech debt (after Phase 3b)

1. **No command merging** — rapid edits create many undo entries.
   Phase 5 can add merge logic.
2. **No edit persistence** — custom styles/edits lost on file
   close. Phase 4 adds session persistence.

## Phase 4 additions — Save and Export

### New core/io/ submodule

Three classes in `src/lumen/core/io/`:

- `WorkspaceFile` — serializes/deserializes PlotScene state to
  JSON sidecar (.lumen.json). Schema v1. See ADR-025.
- `WorkspaceManager` — tracks modification state per document.
  Listens to CommandBus::commandExecuted and DocumentRegistry::
  documentOpened. Provides save/load/revert API.
- `FigureExporter` — renders PlotScene to PNG/SVG/PDF via
  PlotRenderer::render() with different QPainter targets. See
  ADR-026.

### WorkspaceManager position

```
CommandBus::commandExecuted ──→ WorkspaceManager marks modified
DocumentRegistry::documentOpened ──→ WorkspaceManager auto-loads sidecar
PlotRegistry ──→ WorkspaceManager resolves doc → PlotScene
```

### Save/load data flow

```
Open CSV → FileLoader → DocumentRegistry
  → PlotCanvasDock creates default PlotScene
  → WorkspaceManager::loadWorkspaceIfExists
    → if sidecar: WorkspaceFile::loadFromPath → applyToScene
```

### Modified tracking

```
Edit → Command → CommandBus::execute
  → commandExecuted signal
  → WorkspaceManager marks modified
  → modifiedChanged → MainWindow title "● filename"
```

### Export data flow (ADR-026)

```
File → Export Figure → ExportDialog
  → FigureExporter::exportFigure(scene, options)
  → PNG: QImage + QPainter + PlotRenderer::render
  → SVG: QSvgGenerator + QPainter + PlotRenderer::render
  → PDF: QPdfWriter + QPainter + PlotRenderer::render
```

Single rendering code path. Synchronous (ADR-027).

### Layering

- `core/io/` depends on `data/`, `plot/`, `core/`
- `ui/` depends on `core/io/` for save/export
- `plot/` and `data/` unchanged

### Resolved tech debt (cumulative after Phase 4)

| Debt | Resolution | Phase | ADR |
|------|-----------|-------|-----|
| Inline interaction | InteractionController | 3a | ADR-016→020 |
| No hit-testing | HitTester | 3a | ADR-017→019 |
| No undo/redo | CommandBus | 3a | ADR-006→018 |
| Hardcoded margins | computeMargins() | 3b | ADR-013→022 |
| Cursor crosshair | hitTestPoint() | 3b | ADR-017→T6.5 |
| No edit persistence | WorkspaceManager + sidecar | 4 | ADR-025 |

### Remaining tech debt (after Phase 4)

1. **No command merging** — rapid edits create many undo entries.
2. **OutsideRight legend margin** — partial.

## Phase 5 additions — Scatter and Bar Plots

### PlotItem abstraction (ADR-028)

```
PlotItem (abstract base)
  ├── LineSeries    — polyline rendering, segment hit-test
  ├── ScatterSeries — marker rendering (6 shapes), point hit-test
  └── BarSeries     — rectangle rendering, rect hit-test
```

Each subclass implements:
- `type()` → Type enum (Line/Scatter/Bar)
- `paint(QPainter*, CoordinateMapper, plotArea)` — self-rendering
- `dataBounds()` → QRectF
- `isVisible()`, `name()`, `primaryColor()` — for legend/range

Type-specific properties (lineWidth, markerShape, barWidth) stay
on concrete classes, not on PlotItem.

### PlotScene container change

```
PlotScene
  items_: vector<unique_ptr<PlotItem>>
  addItem(unique_ptr<PlotItem>)  ← primary
  addSeries(LineSeries)          ← backward compat wrapper
  itemAt(index) → PlotItem*
```

### Polymorphic rendering (preserves ADR-026)

PlotRenderer iterates `scene.items()` and calls `item->paint()`.
No type switch needed. Legend uses `item->primaryColor()` and
`item->name()` with type-specific swatches:
- Line: 20px line segment
- Scatter: filled marker of series shape
- Bar: filled 12x8 rectangle

### HitTester dispatch for dialogs

Double-click on a PlotItem checks `item->type()`:
- Type::Line → LinePropertyDialog
- Type::Scatter → ScatterPropertyDialog
- Type::Bar → BarPropertyDialog

### New commands (bundled pattern from Phase 3a)

- `ChangeScatterPropertiesCommand` — color, markerShape, markerSize,
  filled, name, visible
- `ChangeBarPropertiesCommand` — fillColor, outlineColor, barWidth,
  name, visible

### ColumnPicker extension

"Plot type" combo (Line/Scatter/Bar) above X/Y selectors. "Add
Series" creates the selected type. Type is immutable after creation
(ADR-029).

### Workspace format (additive, ADR-025 v1)

Series entries gain a "type" field: "line", "scatter", or "bar".
Missing "type" defaults to "line" for backward compat with Phase 4.
Type-specific fields: scatter has markerShape/markerSize/filled;
bar has fillColor/outlineColor/barWidth.

### Layering unchanged

- `plot/` contains PlotItem, LineSeries, ScatterSeries, BarSeries
- `core/commands/` contains type-specific commands
- `ui/` contains type-specific dialogs
- No new cross-layer dependencies

## Phase 6 additions — Universal Data Foundation

### data/ module restructured (ADR-032)

```
data/
  Dataset.h         — abstract base (QObject, reactive signals)
  Dimension.h       — name + unit + length + CoordinateArray
  Unit.h/.cpp       — SI dimensional analysis, parse("mV")
  CoordinateArray.h — regular or irregular coordinate values
  Rank1Dataset.h    — concrete rank-1 (replaces Column)
  TabularBundle.h   — groups rank-1 Datasets (replaces DataFrame)
  Grid2D.h          — rank-2 scalar field
  Volume3D.h        — rank-3 scalar field
  MemoryManager.h   — hybrid memory (ADR-033)
  io/
    DatasetLoader.h  — abstract loader interface (ADR-037)
    LoaderRegistry.h — extension → loader mapping
    CsvLoader.h      — wraps Phase 1 CsvReader
    Hdf5Loader.h     — HDF5 via hdf5 C++ API
    NetCDFLoader.h   — netcdf-c++
    ParquetLoader.h  — Arrow C++
    ZarrLoader.h     — chunked array access
    TiffStackLoader.h — image stacks → Volume3D
    JsonLoader.h     — structured JSON → TabularBundle
    MatLoader.h      — MATLAB .mat files
    NumpyLoader.h    — .npy binary format
```

### DataFrame deleted (ADR-036)

DataFrame and Column classes fully removed. All callers migrated to
TabularBundle (groups of Rank1Datasets sharing a row dimension).
DocumentRegistry stores TabularBundle or Dataset.

### Reactive signals (ADR-035)

Dataset (QObject) emits changed() and coordinatesChanged(). Built
in from Phase 6; Phase 7 plot engine connects for auto-refresh.

### Physical units (ADR-034)

Unit class: 7 SI base dimensions, parse/convert/arithmetic. Grammar:
"m" = meter, "mV" = millivolt, "mm" = millimeter. Axis labels
auto-generated from units.

### Memory model (ADR-033)

MemoryManager singleton: < 100 MB → InMemory, ≥ 100 MB → Chunked
with LRU. Budget: default 4 GB, user-configurable. Status bar
shows usage.

### I/O architecture (ADR-037)

DatasetLoader interface + LoaderRegistry singleton. 9 built-in
loaders. Plugin-ready for Phase 16.

### Layering

- `data/` contains Dataset, all subclasses, Unit, MemoryManager
- `data/io/` contains loaders + registry (depends on data/)
- `core/` DocumentRegistry generalized to Dataset
- `core/io/` WorkspaceFile updated for new API
- `plot/` PlotItem subclasses reference Rank1Dataset
- `ui/` unchanged structurally

## Phase 7 additions — Reactive Plot Engine + 2D Scalar Fields

### core/reactive/ new submodule (ADR-038)

Three-mode reactivity system, user-selectable per plot:

```
ReactiveMode::Static
  → ReactiveBinding deep-copies Dataset into snapshot
  → PlotItem reads snapshot (frozen)
  → MemoryManager tracks 2× allocation
  → invalidate() re-snapshots

ReactiveMode::DAG
  → Dataset::changed() → DependencyGraph::propagate()
  → PlotItem::invalidate() → PlotCanvas::update()
  → PlotItem reads live Dataset (no copy)

ReactiveMode::Bidirectional
  → DAG + write-back interceptor
  → CommandBus edit → Dataset mutation
  → Generation counter prevents feedback loop
```

Classes:
- `DependencyGraph` — DAG of Dataset derivations, propagate,
  cycle detection, generation counter
- `ReactiveBinding` — per-plot binding, mode management,
  snapshot lifecycle, MemoryManager integration

### plot/ new types (ADR-028 extended)

PlotItem Type enum extended:
```
Line, Scatter, Bar,           // Phase 5
Heatmap, Contour,             // Phase 7.2/7.3
Histogram, BoxPlot, Violin    // Phase 7.4
```

**Heatmap** (ADR-039): renders Grid2D via Colormap. Adaptive
CPU/GPU with 1024×1024 threshold. CPU path: QImage pixel loop.
GPU path: QOpenGLWidget texture + fragment shader.

**Colormap** (ADR-040): 10 built-in maps with perceptual
uniformity check (CIELAB ΔE₂₀₀₀ CV < 0.4) and CVD safety
(Machado 2009 simulation).

**ContourPlot** (ADR-041): CONREC-grade contour extraction from
Grid2D. Triangular subdivision for saddle handling. Auto/manual
levels. Labels.

**HistogramSeries** (ADR-042): Scott/FD/Sturges binning,
Count/Density/Probability normalization.

**BoxPlotSeries**: Tukey/MinMax/Percentile whiskers, notched,
outliers.

**ViolinSeries**: Silverman KDE, split-violin for pairs.

### PlotCanvas GPU layer (ADR-039)

```
PlotCanvas
  ├── PlotRenderer (CPU items via QPainter)
  ├── QOpenGLWidget (GPU layer, lazy creation)
  │     └── Heatmap textures, colormap LUT
  └── QPainter overlays (crosshair, zoom box, on top)
```

GPU widget: child of PlotCanvas, positioned at plotArea,
transparent background. Created on first GPU-path Heatmap.
Fallback to CPU if GL context unavailable.

### Reactive data flow

```
Dataset::changed()
  → ReactiveBinding (mode-filtered)
    Static: no-op (snapshot isolated)
    DAG: DependencyGraph::propagate() → downstream invalidation
    Bidir: DAG + write-back interceptor
  → PlotItem::invalidate()
  → PlotCanvas::update()
  → paintEvent → PlotRenderer → item->paint()
```

### ReactivityModeWidget

Small 3-position toggle in property dialogs. Static / DAG / Bidir.
Mode persisted in workspace files.

## Phase 8 additions — Modern 3D Engine

### New plot3d/ submodule (ADR-045: separate from 2D plot/)

```
plot3d/
  PlotItem3D.h        — abstract base (Type, render, hitTestRay, dataBounds)
  PlotCanvas3D.h      — QOpenGLWidget + QOpenGLFunctions_4_5_Core
  Scene3D.h           — vector<PlotItem3D>, lights, bounds
  Camera.h            — Trackball + Orbit modes (ADR-044)
  Light.h             — Directional, Point, Ambient
  Renderer3D.h        — GL state, shader dispatch, per-item render
  ShaderProgram.h     — GLSL compilation + uniform management
  Ray.h               — fromScreenPixel for 3D picking
  BoundingBox3D.h     — AABB for scene bounds
  RenderContext.h     — viewport, matrices, lights for render call
  Scatter3D.h         — instanced rendering, spatial grid hit-test
  SpatialGrid3D.h     — uniform grid for ray-cast acceleration
  Surface3D.h         — Grid2D → mesh, normals, Phong/PBR
  VolumeItem.h        — ray marching shader, transfer function LUT
  TransferFunction.h  — control points → 1D RGBA texture
  Streamlines.h       — RK4 integration, line strip rendering
  Isosurface.h        — Marching Cubes mesh extraction
  MarchingCubes.h     — Lorensen & Cline 1987 algorithm
  PbrMaterial.h       — metallic/roughness/IOR/emissive
  shaders/
    phong.vert/.frag  — Phong lighting (8.1 default)
    pbr.vert/.frag    — Cook-Torrance PBR (8.6)
    volume.vert/.frag — ray marching (8.4)
```

### Hierarchy: PlotItem (2D) ∥ PlotItem3D (3D) (ADR-045)

Two fully separate hierarchies. No shared base class.

| Aspect | 2D (PlotItem) | 3D (PlotItem3D) |
|--------|---------------|-----------------|
| Render | QPainter + CoordinateMapper | ShaderProgram + RenderContext |
| Hit-test | pixel distance | ray cast |
| Bounds | QRectF | BoundingBox3D |
| Canvas | PlotCanvas | PlotCanvas3D |
| Workspace | "plot" block | "plot3d" block |

CommandBus, ReactiveBinding, WorkspaceFile handle both via
per-type dispatch (no unification needed).

### Render flow

```
PlotCanvas3D::paintGL()
  → Renderer3D::render(scene, camera, viewport)
    → for each PlotItem3D:
        → select shader (Phong or PBR per item's material)
        → bind shader, set uniforms (MVP, lights, material)
        → item->render(shader, context)
```

### Reactive flow for 3D items

Same pattern as 2D (ADR-038): Dataset::changed() →
ReactiveBinding (mode-filtered) → PlotItem3D::invalidate() →
PlotCanvas3D::update().

**Debounce for expensive items**: Surface3D and Isosurface set a
dirty flag on invalidate(); actual mesh regeneration is debounced
to 200ms via QTimer.

**Volume3D Static mode**: lazy snapshot (reference + generation
stamp) instead of deep copy. No 2× memory cost for GB-scale
volumes.

### Camera system (ADR-044)

```
Left-drag → Camera::handleDrag(delta)
  Trackball: arcball quaternion rotation (free 6-DOF)
  Orbit: azimuth + elevation around target (constrained)

Wheel → Camera::handleWheel(delta)
  Both: dolly zoom toward/away from target

Middle-drag → Camera::handlePan(delta)
  Both: translate target in screen plane
```

### Hit-testing (ADR-048)

```
Double-click pixel → Ray::fromScreenPixel(pixel, camera, viewport)
  → for each PlotItem3D: hitTestRay(ray, maxDist)
    Scatter3D: SpatialGrid3D DDA traversal → ray-sphere
    Surface3D: ray-triangle on mesh
    VolumeItem: bounds check → dialog
    Streamlines: ray-cylinder
    Isosurface: ray-triangle
  → closest hit → open type-specific dialog
```

### Lighting (ADR-047)

Phong default + PBR opt-in per PlotItem3D.

### Workspace format extension

```json
{
  "plot": { /* 2D, unchanged */ },
  "plot3d": {
    "camera": { "mode": "trackball", "position": [...], "target": [...] },
    "lights": [...],
    "items": [
      { "type": "scatter3d", /* properties */ },
      { "type": "surface3d", "material": { "type": "pbr", ... } },
      { "type": "volume", "transferFunction": { "points": [...] } }
    ]
  }
}
```

### Layering

- `plot3d/` depends on `data/`, `plot/Colormap`, `core/`
- `ui/` depends on `plot3d/` for 3D dialogs
- `plot/` unchanged — 2D code untouched
- `core/reactive/` unchanged — ReactiveBinding works for both
