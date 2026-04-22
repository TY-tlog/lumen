# Phase 12 — Dashboard Integration + Multi-File Workflows

**Status**: spec
**Predecessor**: Phase 11 (vphase-11, f83044c)
**Successor**: Phase 13 (Live Data Streaming + Protocol Adapters)
**Scope**: Wire Phase 11 dashboard into MainWindow, multi-file loading,
dashboard-level export, tab-based multi-dashboard, auto-scroll linking.

---

## 1. Motivation

Phase 11 built dashboard infrastructure (model, layout, linking,
persistence, commands) but did not wire it into MainWindow. The
owner cannot create or use dashboards from the application UI.
Additionally, the owner's data is spread across hundreds of CSV
files (one per calcium concentration), and there is no way to load
multiple files into a single dashboard view.

## 2. Sub-phases

| Sub  | Title                           | Core deliverable                          |
|------|---------------------------------|-------------------------------------------|
| 12.1 | MainWindow Dashboard Mode       | View toggle, DashboardWidget integration  |
| 12.2 | Multi-File Data Loading         | Load N files into N dashboard panels      |
| 12.3 | Dashboard Export                 | Multi-panel PNG/SVG/PDF export            |
| 12.4 | Auto-Scroll Linking             | Time-axis auto-scroll for streaming data  |

## 3. Sub-phase 12.1 — MainWindow Dashboard Mode

### Goal
Add View > Dashboard Mode toggle to MainWindow. When activated,
replace the single-plot vSplitter with a DashboardWidget. The
dashboard toolbar appears at the top. Panel add/remove works via
UI.

### Design
- View menu gains "Dashboard Mode" checkable action
- Toggle ON: create Dashboard + DashboardWidget, replace vSplitter
  content. DataTableDock shows the selected panel's data.
- Toggle OFF: restore single-plot view (non-destructive: dashboard
  object persists)
- First panel auto-created from current data (if any loaded)

### Deliverables
- MainWindow dashboard mode toggle wiring
- DashboardWidget lifecycle management in MainWindow
- 5 new tests

## 4. Sub-phase 12.2 — Multi-File Data Loading

### Goal
Enable loading multiple data files into a dashboard, each in its
own panel. Support the owner's workflow: "load all calcium
concentration CSVs into a 2×4 grid."

### Design
- File > Open Multiple... action (QFileDialog multi-select)
- Each selected file loads into a new dashboard panel
- Auto-creates dashboard grid from file count (ceil(sqrt(N)) cols)
- Each panel title = filename
- X-axis linking enabled by default (time alignment)

### Deliverables
- Multi-file open dialog and loading logic
- Auto-grid creation from file count
- 5 new tests

## 5. Sub-phase 12.3 — Dashboard Export

### Goal
Export the full dashboard as a single multi-panel figure.

### Design
- File > Export Dashboard... action
- FigureExporter extended with dashboard export
- Each panel rendered into its grid sub-rect using PlotRenderer
- Supports PNG, SVG, PDF (same as single-plot export)

### Deliverables
- FigureExporter::exportDashboard()
- ExportDialog dashboard mode
- 4 new tests + 1 smoke test (S13)

## 6. Sub-phase 12.4 — Auto-Scroll Linking

### Goal
When linked panels show time-series data, enable an auto-scroll
mode where the X-axis window slides to follow new data. Used for
reviewing long recordings.

### Design
- LinkGroup gains AutoScroll channel
- When enabled, all linked panels keep a fixed X-axis window width
  and scroll to show the latest data point
- Manual pan temporarily disables auto-scroll (user override)
- Re-enable via toolbar button

### Deliverables
- LinkGroup auto-scroll channel
- Toolbar auto-scroll toggle
- 5 new tests
