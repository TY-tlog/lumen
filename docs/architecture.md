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
