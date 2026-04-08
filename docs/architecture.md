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
