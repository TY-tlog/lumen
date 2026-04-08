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
