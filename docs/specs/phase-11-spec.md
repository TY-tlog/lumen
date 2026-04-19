# Phase 11 — Dashboard + Linked Views

**Status**: spec
**Predecessor**: Phase 10.5 (vphase-10.5, 98d0a95)
**Successor**: Phase 12 (Live Data Streaming + Protocol Adapters)
**Scope**: Multi-plot dashboard layout, view linking (pan/zoom/crosshair/selection), dashboard-level theme cascade, workspace format v2 for dashboard persistence, dashboard editor UI.

---

## 1. Motivation

The owner (T.Y.) works with electrophysiology patch-clamp data:
voltage, current, and capacitance traces recorded simultaneously.
The single-plot window forces switching between columns or opening
multiple instances. There is no way to:

- View voltage and current side-by-side with synchronized X axes
- Pan one trace and have a linked trace follow automatically
- Compare pre/post drug application traces with aligned time axes
- Arrange a publication-ready multi-panel figure (Fig 1A, 1B, 1C)
- Apply a consistent theme across related panels

## 2. Scope

### In scope
- Dashboard data model: container owning multiple PlotScene instances
- Grid-based layout engine (rows × columns, cell spanning)
- View linking: 4 channels (X-axis, Y-axis, crosshair, selection)
- Dashboard as preset level in the 4-level style cascade
- Workspace format v2: backward-compatible extension
- Dashboard editor UI: panel add/remove, layout, link configuration
- Dashboard-level export (multi-panel figure as single PNG/SVG/PDF)
- 8 new smoke tests (S6–S13)

### Out of scope (deferred)
- Free-form floating panels (Phase 14)
- Tab-based multi-dashboard (Phase 12+)
- 3D panels in dashboard (Phase 11.5 or 12)
- Cross-dashboard linking
- Dashboard templates/presets (Phase 12+)
- Drag-and-drop panel reordering (Phase 11.5)
- Live data streaming (Phase 12)
- OS auto light/dark switch (still deferred)

## 3. Sub-phases

| Sub  | Title                    | Core deliverable                            |
|------|--------------------------|---------------------------------------------|
| 11.1 | Dashboard Model + Layout | Dashboard, PanelConfig, GridLayout, Widget  |
| 11.2 | View Linking Engine      | LinkGroup, 4 link channels, propagation     |
| 11.3 | Theme Cascade Integration| Dashboard as preset level, per-panel override|
| 11.4 | Persistence (Workspace v2)| Serialization, v1→v2 migration             |
| 11.5 | Dashboard Editor UI      | Toolbar, dialogs, commands, mode toggle     |
| 11.6 | Export + Smoke Tests     | Multi-panel export, S6–S13                  |

Each sub-phase is an M-gate. Zero regression on prior 840 tests.

## 4. Sub-phase 11.1 — Dashboard Model + Layout

### Data model

```cpp
namespace lumen::dashboard {

struct PanelConfig {
    int row, col;
    int rowSpan = 1, colSpan = 1;
    QString title;
    QString linkGroup = "default";
};

class Dashboard : public QObject {
    Q_OBJECT
public:
    int addPanel(PanelConfig config);
    void removePanel(int index);
    int panelCount() const;
    PanelConfig& panelConfigAt(int index);
    plot::PlotScene* sceneAt(int index);
    void setGridSize(int rows, int cols);
    void setDashboardStyle(const style::Style& style);
signals:
    void panelAdded(int index);
    void panelRemoved(int index);
    void layoutChanged();
};
}
```

### GridLayout engine

Divides available area by grid dimensions, applies spacing (8px).
Max grid: 8×8. No overlap detection — editor prevents conflicts.

### DashboardWidget

Plain QWidget (not QDockWidget). Each PlotCanvas is a direct child
positioned via setGeometry. Avoids QOpenGLWidget nesting issues
from Phase 10.5.

### Deliverables

| File | Owner |
|------|-------|
| `src/lumen/dashboard/Dashboard.h/.cpp` | Backend |
| `src/lumen/dashboard/PanelConfig.h` | Backend |
| `src/lumen/dashboard/GridLayout.h/.cpp` | Backend |
| `src/lumen/dashboard/DashboardWidget.h/.cpp` | Frontend |
| `tests/unit/test_dashboard_model.cpp` | QA |
| `tests/unit/test_grid_layout.cpp` | QA |
| ADR-070, ADR-071 | Architect |

### M11.1 gate
Human creates a 2×2 dashboard, sees 4 panels resize correctly.
"Human response:" recorded with screenshot.

## 5. Sub-phase 11.2 — View Linking Engine

### Link channels

| Channel   | Propagates                     | Source event         |
|-----------|--------------------------------|----------------------|
| X-axis    | ViewTransform X range          | PlotViewportChanged  |
| Y-axis    | ViewTransform Y range          | PlotViewportChanged  |
| Crosshair | Cursor data-space X position   | PlotCrosshairMoved   |
| Selection | Selected data range            | SelectionChanged     |

### Feedback prevention

Generation counter on LinkGroup: each propagation increments
generation. Receiving panel checks generation before re-emitting.
Same pattern as Phase 7 DependencyGraph.

### Default configuration

X-axis + crosshair linked, Y-axis independent (voltage and current
have different scales). This matches electrophysiology workflow.

### M11.2 gate
Human opens 2-panel dashboard with voltage + current from the same
CSV. Pans voltage, current follows. Hovers voltage, crosshair
appears in current. "Human response:" recorded.

## 6. Sub-phase 11.3 — Theme Cascade Integration

Dashboard becomes the "preset" level in the 4-level cascade:

| Level          | Maps to                    |
|----------------|----------------------------|
| Theme          | Global active theme        |
| **Preset**     | **Dashboard style**        |
| Plot instance  | Panel-specific overrides   |
| Element        | Per-element edits          |

Standalone plots unaffected (empty preset, as before).

### M11.3 gate
Human sets dashboard background to dark gray. All panels reflect.
Overrides one panel to white. StyleInspector shows sources correctly.

## 7. Sub-phase 11.4 — Persistence (Workspace v2)

### Schema extension

```json
{
  "lumen_workspace_version": 2,
  "dashboard": {
    "gridRows": 2, "gridCols": 2,
    "style": {},
    "linkGroups": [{"name": "default", "channels": ["x-axis", "crosshair"]}],
    "panels": [{"row": 0, "col": 0, "plot": {}}]
  },
  "dataSources": [{"path": "data.csv", "format": "csv"}]
}
```

V1 files auto-migrate: single plot wrapped in 1×1 dashboard.

### M11.4 gate
Human saves 2×2 dashboard, closes, reopens. Layout + links + styles
restored. Also opens a v1 workspace — loads as 1×1 dashboard.

## 8. Sub-phase 11.5 — Dashboard Editor UI

### Components

- **DashboardToolbar**: `[+ Add Panel] [Grid: 2×2 ▾] [Link Editor] [Dashboard Style]`
- **AddPanelDialog**: data source, X/Y column, plot type, position
- **LinkEditorDialog**: group management, channel toggles, member list
- **Panel context menu**: edit, change source, link group, remove
- **MainWindow toggle**: View → Dashboard Mode (non-destructive)

### Commands (undo/redo)

AddDashboardPanelCommand, RemoveDashboardPanelCommand,
ChangeDashboardLayoutCommand, ChangeLinkGroupCommand.

### M11.5 gate
Human creates dashboard from scratch via toolbar. Adds 3 panels,
configures linking, sets grid, undoes panel addition.

## 9. Sub-phase 11.6 — Export + Smoke Tests

### Multi-panel export

Extends FigureExporter: render each PlotScene into its grid sub-rect
using PlotRenderer. Reuses existing single-path rendering.

### Smoke tests (S6–S13)

| ID  | Scenario                     | Assertion                              |
|-----|------------------------------|----------------------------------------|
| S6  | Dashboard mode toggle        | Window visible after toggle            |
| S7  | Add 2 panels to 1×2 grid    | Both have visual content (pixel var)   |
| S8  | X-axis link: pan panel 0     | Panel 1 viewport matches              |
| S9  | Crosshair sync               | Cursor line in linked panel            |
| S10 | Dashboard theme set          | Both panels reflect new background     |
| S11 | Dashboard save/load          | Round-trip preserves layout + links    |
| S12 | Dashboard export (PNG)       | Output has content, correct size       |
| S13 | V1 workspace in dashboard    | Loads as 1×1 dashboard                 |

### M11.6 gate
Human exports 2×3 dashboard as PNG and SVG. All panels visible in
external viewer. All 8 smoke tests pass.

## 10. Task breakdown summary

39 tasks across 6 sub-phases. ~115 new tests. 840 → ~955 total.
Estimated 4–6 weeks calendar.

## 11. Hard rules

1. Review-in-same-commit
2. Per-deliverable verification (no fabricated claims — Phase 8 lesson)
3. Branch check before every commit
4. Zero prior regression (840 tests)
5. M-gate human verification with screenshot + "Human response:"
6. Smoke tests use hasVisualContent() pixel checks (ADR-069)
7. Shader routing simplicity (Phase 10.5 lesson)

## 12. Phase 12 hand-forward

Phase 12 needs: dashboard as primary display container (built),
streaming data binding (API ready: Dashboard::bindDataset),
link propagation with auto-scroll mode, OS theme switch,
tab-based multi-dashboard.
