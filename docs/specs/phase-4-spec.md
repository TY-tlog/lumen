# Phase 4 — Save and Export

## Goal

Transform Lumen from "edit in the moment" to "edit, save, reopen,
export". After this phase, the user can open a CSV, make edits,
close Lumen, reopen the same CSV, and see their edits restored;
and can export the current plot as PNG, SVG, or PDF for
publication use.

This is the phase that makes Lumen an end-to-end tool instead of
a viewer-with-editing.

## Active agents

Architect, Backend, Frontend, QA, Integration, Docs.

## Scope: two internal sub-phases

Phase 4 is delivered as two sub-phases with a merge and verification
point between them. The human can stop after 4.1 if 4.1 alone meets
their need.

- **Phase 4.1**: Edit persistence (workspace file)
- **Phase 4.2**: Figure export (PNG/SVG/PDF)

Both sub-phases are additive to existing modules. No refactoring
of Phase 3a/3b code.

## Phase 4.1 — Edit Persistence

### User-visible deliverables

1. After editing a plot (line color, axis labels, title, legend,
   viewport, etc.), closing and reopening the same CSV in Lumen
   restores all edits exactly.
2. Edits are stored in a sidecar file next to the CSV:
   `<filename>.lumen.json` in the same directory. No change to
   the CSV itself.
3. File menu gains: "Save Workspace" (Ctrl+S) and "Save Workspace
   As..." — the latter lets user pick a path other than the
   default sidecar.
4. Auto-save on close: when user closes a document or quits
   Lumen with unsaved edits, Lumen prompts "Save workspace for
   <filename>?" (Save / Discard / Cancel).
5. Status bar shows modification indicator ("●" dot) when there
   are unsaved edits since last save.
6. File menu gains "Revert to Saved" that discards edits and
   reloads the saved workspace (or the unedited default if no
   saved workspace).

### Technical deliverables

#### core/io/WorkspaceFile.{h,cpp}

Read/write a .lumen.json sidecar. JSON schema version 1:
{ "version": 1, "csv_path": "relative/or/absolute/path/to/file.csv", "plot": { "viewport": { "xmin": ..., "xmax": ..., "ymin": ..., "ymax": ... }, "title": { "text": "...", "fontPx": 14, "weight": 600 }, "xAxis": { "label": "Time (ms)", "rangeMode": "manual", "manualMin": 0.0, "manualMax": 1.2, "tickCount": 0, "tickFormat": "auto", "gridVisible": true }, "yAxis": { "label": "Vm (mV)", "rangeMode": "auto", "tickCount": 0, "tickFormat": "auto", "gridVisible": true }, "legend": { "position": "top_right", "visible": true }, "series": [ { "xColumn": 0, "yColumn": 1, "color": "#0a84ff", "lineWidth": 1.5, "lineStyle": "solid", "name": "Voltage", "visible": true } ] } }

WorkspaceFile class:
class WorkspaceFile { public: static WorkspaceFile loadFromPath(const QString& path); void saveToPath(const QString& path) const;

// Populated from current plot state
static WorkspaceFile captureFromScene(const PlotScene* scene);

// Apply to a freshly-loaded plot
void applyToScene(PlotScene* scene) const;

bool isValid() const;
QString version() const;
private: // Internal representation (QJsonDocument or struct) };

#### core/io/WorkspaceManager.{h,cpp} Tracks workspace file state per open document:
class WorkspaceManager : public QObject { Q_OBJECT public: explicit WorkspaceManager(DocumentRegistry* docs, PlotRegistry* plots, CommandBus* bus, QObject* parent = nullptr)

// Returns true on success
bool saveWorkspace(const QString& docPath);
bool saveWorkspaceAs(const QString& docPath, const QString& outPath);
bool loadWorkspaceIfExists(const QString& docPath);
bool revertToSaved(const QString& docPath);

bool isModified(const QString& docPath) const;
QString defaultSidecarPath(const QString& docPath) const;

signals: void modifiedChanged(const QString& docPath, bool modified); void workspaceSaved(const QString& docPath, const QString& outPath); void workspaceLoaded(const QString& docPath, const QString& inPath);
private: // Listens to CommandBus::executed to mark modified // Listens to DocumentRegistry::documentOpened to auto-load sidecar };

#### DocumentRegistry integration

When a document is opened, WorkspaceManager listens and:
1. Looks for sidecar at defaultSidecarPath
2. If present, loads and applies after the PlotCanvas has
   created its default plot
3. If absent, no action (default Phase 2 behavior)

#### MainWindow integration

- File menu: Save Workspace (Ctrl+S), Save Workspace As...,
  Revert to Saved
- Close/quit handler: prompt for unsaved changes
- Window title shows modification indicator
- WorkspaceManager ownership: alongside DocumentRegistry,
  PlotRegistry, CommandBus

#### CommandBus hook

CommandBus::executed signal -> WorkspaceManager marks the relevant
document as modified. The plot-to-document mapping goes through
PlotRegistry (Phase 2.5).

### Tests for Phase 4.1

Unit tests:
- test_workspace_file.cpp (>=6 tests)
  - Save and load roundtrip preserves all fields
  - Version 1 schema validation
  - Invalid JSON returns isValid() false
  - Missing optional fields use defaults
  - Relative vs absolute csv_path handling
  - captureFromScene reflects current PlotScene state
- test_workspace_manager.cpp (>=5 tests)
  - Modified flag set on CommandBus executed
  - Modified flag cleared on save
  - loadWorkspaceIfExists succeeds with existing sidecar
  - loadWorkspaceIfExists is no-op without sidecar
  - revertToSaved restores pre-edit state

Integration tests:
- test_persistence_roundtrip.cpp: open CSV, edit line color,
  save workspace, close document, reopen, verify color restored
- test_sidecar_auto_load.cpp: place a .lumen.json next to a CSV,
  open the CSV, verify edits applied automatically
- test_revert_to_saved.cpp: edit, save, edit again, revert,
  verify state matches first save

## Phase 4.2 — Figure Export

### User-visible deliverables

7. File menu gains: "Export Figure..." (Ctrl+E)
8. Export dialog with:
   - Format: PNG / SVG / PDF radio buttons
   - Size presets: "As shown" (current plot widget size),
     "Publication single column" (3.5 inch = ~1050 px at 300 DPI),
     "Publication double column" (7 inch = ~2100 px at 300 DPI),
     "Custom" (width/height in pixels or inches)
   - DPI: 72 / 150 / 300 / 600 (PNG and PDF only; SVG is vector)
   - Background: White / Transparent (PNG and PDF)
   - File path picker with format-appropriate default extension
9. Export button: renders the plot to the chosen format and saves
10. After successful export, status bar shows "Exported to
    <path>" for 3 seconds
11. Export does NOT change the current plot state; it is purely
    a render-to-file operation

### Technical deliverables

#### core/io/FigureExporter.{h,cpp}

class FigureExporter { public: enum class Format { Png, Svg, Pdf };

struct Options {
    Format format = Format::Png;
    int widthPx = 1050;
    int heightPx = 700;
    int dpi = 300;
    bool transparentBackground = false;
    QString outputPath;
};

// Renders the given PlotScene to a file.
// Returns empty string on success, error message on failure.
static QString exportFigure(const PlotScene* scene,
                             const Options& opts);
private: static QString exportPng(const PlotScene*, const Options&); static QString exportSvg(const PlotScene*, const Options&); static QString exportPdf(const PlotScene*, const Options&); };

Implementation notes:
- PNG: QImage with specified size, QPainter renders PlotScene to
  it via a temporary CoordinateMapper scaled to the target size
- SVG: QSvgGenerator, same render path
- PDF: QPdfWriter, same render path
- All three reuse the existing PlotRenderer::render() with a
  different QPainter target, so there is exactly one rendering
  code path — no separate "export renderer"
- Font embedding is automatic for SVG and PDF via Qt
- Transparent background: render without the background fill step

#### ui/ExportDialog.{h,cpp}

Non-modal QDialog with format/size/DPI/background controls per
spec. Apple-mood styled. On Export button click, constructs a
FigureExporter::Options and calls exportFigure().

#### MainWindow integration

- File menu: "Export Figure..." (Ctrl+E)
- Opens ExportDialog with current PlotScene from active document
- Status bar message on success/failure

### Tests for Phase 4.2

Unit tests:
- test_figure_exporter.cpp (>=8 tests)
  - PNG export produces a file at the specified size
  - PNG at different DPIs produces different pixel counts
  - SVG export produces a valid SVG (check for <svg> root)
  - PDF export produces a valid PDF (check for %PDF- header)
  - Transparent background PNG has no opaque background pixel
    at (0,0)
  - Export does not modify PlotScene state (capture scene hash
    before and after)
  - Invalid output path returns error message
  - Empty PlotScene (no series) still exports without crash

Integration tests:
- test_export_png_roundtrip.cpp: open sine fixture, export to
  PNG, verify file exists and has expected size
- test_export_svg_contains_series.cpp: export sine to SVG,
  parse SVG, verify <path> element with the series color
- test_export_preserves_edits.cpp: edit line color, export,
  verify the exported file reflects the edit

## ADRs

- ADR-025 Workspace file format (.lumen.json sidecar v1)
  - Alternatives: embed in CSV (rejected: modifies user data),
    central workspace DB (rejected: breaks copy-file portability),
    binary format (rejected: not human-readable)
- ADR-026 Figure export reuses PlotRenderer
  - Alternatives: separate export renderer (rejected: two code
    paths diverge), pixmap grab from widget (rejected: limited to
    widget size and raster)
- ADR-027 Non-blocking export vs modal export
  - Phase 4 uses modal-lite: dialog is non-modal but export
    itself runs synchronously in main thread. Alternatives:
    background thread with progress (deferred to Phase 5+ if
    exports become slow on large plots)

## Architecture updates

docs/architecture.md gets a Phase 4 additions section:
- New core/io/ submodule containing WorkspaceFile,
  WorkspaceManager, FigureExporter
- WorkspaceManager position: between DocumentRegistry and
  CommandBus, listens to both
- Save/load data flow: DocumentRegistry.documentOpened ->
  WorkspaceManager.loadWorkspaceIfExists -> WorkspaceFile.load ->
  applyToScene -> PlotScene
- Export data flow: MainWindow -> ExportDialog -> FigureExporter
  -> PlotRenderer.render(QPainter*)
- Layering: core/io/ depends on data/, plot/, core/. ui/ depends
  on core/io/ for MainWindow integration.

## Acceptance criteria

./scripts/dev_build.sh # zero warnings, zero errors 
./scripts/dev_test.sh # all tests pass under ASan+UBSan 
./scripts/dev_run.sh # manual flow below

Manual flow:

Phase 4.1 verification:
1. Open a real electrophysiology CSV
2. Edit line color to red
3. Edit X axis label to "Time (ms)"
4. Edit title to "d20251029_s002"
5. Ctrl+S saves workspace
6. Verify <filename>.lumen.json appears next to the CSV
7. Close the document (or quit Lumen)
8. Reopen the same CSV
9. Verify line is red, X axis says "Time (ms)", title says
   "d20251029_s002"
10. Edit something else, do NOT save
11. Close: prompted to save
12. Click Discard: edit gone on reopen

Phase 4.2 verification:
13. With edited plot open, File -> Export Figure... (Ctrl+E)
14. Select PNG, 300 DPI, Publication single column, Save
15. Verify PNG file exists at chosen path, size is ~1050x700 px
16. Open PNG in an image viewer: plot looks correct
17. Export again as SVG: verify file exists, open in browser,
    verify vector rendering
18. Export as PDF: verify file exists, open in PDF viewer,
    verify plot renders

## Real-data exit criterion

Owner opens real electrophysiology CSV, edits to publication
style (colors, axis labels, title, range), saves workspace,
exports as PDF at 300 DPI, and confirms:

- [ ] The saved workspace reopens with all edits intact
- [ ] The exported PDF is visually identical to what Lumen shows
- [ ] The PDF could be placed in a manuscript without further
      tool work
- [ ] Export + save workflow feels natural (no surprising modal
      blocks, no lost edits)

Phase 3a and 3b regression:
- [ ] Line editing still works (Phase 3a)
- [ ] Axis/title/legend editing still works (Phase 3b)
- [ ] Pan, zoom, crosshair still work (Phase 2)
- [ ] 217 pre-existing tests still pass

## Non-goals (deferred to later phases)

- Scatter/bar plot types (Phase 5)
- Dark theme (Phase 6)
- Multi-plot / subplot (Phase 7)
- Data decimation / LOD (later)
- Style presets / named saved styles (Phase 5 or later)
- Workspace file format migration (version > 1) (when needed)
- Export animation/video (not planned)
- Background-thread export with progress bar (Phase 5+ if needed)
- Clipboard image export (Phase 5 possible)

## Risks and mitigations

| Risk | Mitigation |
|---|---|
| Workspace sidecar path conflicts with existing files | Use .lumen.json extension which is unlikely to exist; prompt before overwrite on "Save As" |
| Sidecar load fails partway, plot left in inconsistent state | Load into a temp WorkspaceFile, validate, then apply atomically; rollback on error |
| Export at 600 DPI on large plots is slow | Document as known limitation; Phase 5+ background thread if needed |
| SVG font embedding varies by Qt version | Test on Ubuntu Qt 6.4.2 (project's minimum); document any issues |
| PDF rendering differs subtly from on-screen | Use same PlotRenderer code path; accept small rendering differences as inherent to format |
| CommandBus modified-tracking misses direct Plot mutations | All mutations must go through commands; audit Backend to confirm |
| User saves sidecar, then moves the CSV: sidecar orphaned | Accept as user responsibility; sidecar contains csv_path for diagnostics |

## Task breakdown

### Architect (S)
- docs/plans/phase-4-plan.md
- ADR-025, ADR-026, ADR-027
- docs/architecture.md Phase 4 section
- STATUS opening entry

### Backend (L)
Phase 4.1:
- core/io/WorkspaceFile.{h,cpp} + unit tests
- core/io/WorkspaceManager.{h,cpp} + unit tests
- CommandBus::executed connection to WorkspaceManager
- DocumentRegistry hook for auto-load
Phase 4.2:
- core/io/FigureExporter.{h,cpp} + unit tests
Approximately 6-10 hours.

### Frontend (M)
Phase 4.1:
- MainWindow File menu updates (Save, Save As, Revert to Saved)
- Close/quit unsaved-changes prompt
- Window title modification indicator
- Status bar modification dot
Phase 4.2:
- ui/ExportDialog.{h,cpp}
- MainWindow Export Figure... menu entry
- Export success/failure status bar messages
Approximately 4-6 hours.

### QA (M)
- All unit tests listed above
- All integration tests listed above
- Regression check: all 217 pre-existing tests still pass
- Manual verification of Phase 3a/3b regression
Approximately 3-5 hours.

### Integration (S)
- Merge windows between 4.1 and 4.2 (clean separation)
- Update CHANGELOG
- Tag vphase-4 at exit

### Docs (S)
- README update: what Lumen can do now (end-to-end)
- src/lumen/core/io/CLAUDE.md (new submodule)
- docs/reviews/phase-4-review.md at close, WRITTEN IN SAME
  COMMIT AS THE CLOSING STATUS ENTRY (lesson from Phase 3a/3b)

## Exit checklist

Phase 4.1 sub-checklist:
- [ ] WorkspaceFile load/save roundtrip passes
- [ ] WorkspaceManager modified-tracking works
- [ ] Save Workspace menu item functional
- [ ] Auto-load sidecar on document open
- [ ] Revert to Saved functional
- [ ] Unsaved-changes prompt on close
- [ ] All Phase 4.1 tests pass
- [ ] Human verifies edit-save-reopen roundtrip with real CSV

Phase 4.2 sub-checklist:
- [ ] FigureExporter PNG/SVG/PDF all work
- [ ] ExportDialog functional
- [ ] Export does not mutate PlotScene
- [ ] All Phase 4.2 tests pass
- [ ] Human exports one publication-ready figure in each format

Phase 4 overall:
- [ ] Build clean
- [ ] All tests pass under ASan+UBSan
- [ ] Phase 3a/3b/2 regression clean
- [ ] ADR-025, 026, 027 committed
- [ ] docs/reviews/phase-4-review.md committed IN SAME COMMIT as
      closing STATUS entry
- [ ] vphase-4 tag pushed

