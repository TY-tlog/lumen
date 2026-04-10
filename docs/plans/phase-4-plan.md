# Phase 4 Plan — Save and Export

> Reference: `docs/specs/phase-4-spec.md`

## Hard rule from Phase 3b lesson

**docs/reviews/phase-4-review.md MUST be written and committed in
the SAME COMMIT as the closing .lumen-ops/STATUS.md entry.** This
is not a separate task. It is baked into T8 (Docs closing task).
The Docs agent must produce both files in one `git add + git commit`.

## Two sub-phases with gate

Phase 4 is delivered as two sub-phases. Sub-phase 4.2 does NOT
start until sub-phase 4.1 passes human verification. This prevents
exporting from an app that can't even save.

```
Sub-phase 4.1: Edit Persistence
  T1 → T2 → T3 → T4 → M4.1 (human gate)

Sub-phase 4.2: Figure Export
  T5 → T6 → T7 → M4.2 (human gate)

Closing:
  T8 (review + STATUS in SAME commit)
```

---

## Task Dependency Graph

```
  ┌────────────┐    ┌────────────────┐
  │T1 Workspace│    │T2 Workspace    │
  │ File (back)│    │ Manager (back) │
  └─────┬──────┘    └───────┬────────┘
        │                   │
        └─────┬─────────────┘
              ▼
     ┌─────────────────┐
     │T3 MainWindow     │
     │ save/revert UI   │
     │ (frontend)       │
     └────────┬────────┘
              ▼
     ┌─────────────────┐
     │T4 QA 4.1        │
     │ persistence     │
     │ tests           │
     └────────┬────────┘
              ▼
     ═══════════════════
     ║ M4.1 HUMAN GATE ║
     ═══════════════════
              │
     ┌────────┼────────┐
     ▼                 ▼
  ┌────────────┐  ┌────────────────┐
  │T5 Figure   │  │T6 ExportDialog │
  │ Exporter   │  │ (frontend)     │
  │ (backend)  │  │                │
  └─────┬──────┘  └───────┬────────┘
        │                 │
        └─────┬───────────┘
              ▼
     ┌─────────────────┐
     │T7 QA 4.2        │
     │ export tests    │
     └────────┬────────┘
              ▼
     ═══════════════════
     ║ M4.2 HUMAN GATE ║
     ═══════════════════
              │
              ▼
     ┌─────────────────┐
     │T8 Docs closing  │
     │ (review+STATUS  │
     │  SAME commit)   │
     └─────────────────┘
```

---

## Sub-phase 4.1 — Edit Persistence

### T1 — WorkspaceFile (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/core/io/WorkspaceFile.h`
- `src/lumen/core/io/WorkspaceFile.cpp`
- `src/lumen/core/io/CMakeLists.txt` (new static library `lumen_io`)
- `tests/unit/test_workspace_file.cpp`

**Files to modify**:
- `src/lumen/core/CMakeLists.txt` — add `add_subdirectory(io)`
- `src/lumen/CMakeLists.txt` — link `lumen_io` to lumen target
- `tests/unit/CMakeLists.txt` — add test file, link `lumen_io`

**Implementation**:

```cpp
namespace lumen::core::io {

class WorkspaceFile {
public:
    /// Load from a .lumen.json file. Returns invalid WorkspaceFile
    /// if file doesn't exist or JSON is malformed.
    static WorkspaceFile loadFromPath(const QString& path);

    /// Save current state to a .lumen.json file.
    void saveToPath(const QString& path) const;

    /// Capture all serializable state from a PlotScene.
    static WorkspaceFile captureFromScene(const plot::PlotScene* scene);

    /// Apply saved state to a PlotScene (viewport, title, axes,
    /// legend, series styles).
    void applyToScene(plot::PlotScene* scene) const;

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] int version() const;

private:
    QJsonObject data_;
    bool valid_ = false;
};

}
```

JSON schema v1 per spec: version, csv_path, plot.viewport,
plot.title, plot.xAxis, plot.yAxis, plot.legend, plot.series[].

captureFromScene reads: ViewTransform (xMin/xMax/yMin/yMax),
title/titleFontPx/titleWeight, xAxis (label, rangeMode, manualMin,
manualMax, tickCount, tickFormat, tickFormatDecimals, gridVisible),
yAxis (same), legend (position, visible), each LineSeries (xColumn
index, yColumn index, color, lineWidth, lineStyle, name, visible).

applyToScene writes all the above back. Series are matched by
column index (not name) since the user might have renamed them.

**Acceptance criteria**:
- Save + load roundtrip preserves all fields exactly
- Invalid JSON returns isValid() == false
- Missing optional fields use sensible defaults
- captureFromScene reflects current PlotScene state
- 6+ unit tests pass under ASan+UBSan

**Dependencies**: none (day 1)

---

### T2 — WorkspaceManager (Backend, Size: L)

**Owner**: backend

**Files to create**:
- `src/lumen/core/io/WorkspaceManager.h`
- `src/lumen/core/io/WorkspaceManager.cpp`
- `tests/unit/test_workspace_manager.cpp`

**Implementation**:

```cpp
namespace lumen::core::io {

class WorkspaceManager : public QObject {
    Q_OBJECT
public:
    explicit WorkspaceManager(
        core::DocumentRegistry* docs,
        core::PlotRegistry* plots,
        core::CommandBus* bus,
        QObject* parent = nullptr);

    bool saveWorkspace(const QString& docPath);
    bool saveWorkspaceAs(const QString& docPath, const QString& outPath);
    bool loadWorkspaceIfExists(const QString& docPath);
    bool revertToSaved(const QString& docPath);

    [[nodiscard]] bool isModified(const QString& docPath) const;
    [[nodiscard]] QString defaultSidecarPath(const QString& docPath) const;

signals:
    void modifiedChanged(const QString& docPath, bool modified);
    void workspaceSaved(const QString& docPath, const QString& outPath);
    void workspaceLoaded(const QString& docPath, const QString& inPath);
};

}
```

Key behaviors:
- Constructor connects `CommandBus::commandExecuted` → mark
  document as modified. The document is identified by walking
  PlotRegistry to find which document a command's PlotScene
  belongs to.
- Constructor connects `DocumentRegistry::documentOpened` →
  call loadWorkspaceIfExists(path).
- `defaultSidecarPath`: replace CSV extension with `.lumen.json`
  (e.g., `data.csv` → `data.lumen.json`).
- `saveWorkspace`: capture from PlotScene via WorkspaceFile::
  captureFromScene, save to defaultSidecarPath, clear modified
  flag, emit workspaceSaved.
- `revertToSaved`: load workspace from default path, apply to
  scene, clear undo stack (since we're reverting), clear modified.
- Modified tracking: `QHash<QString, bool> modified_`.

**Acceptance criteria**:
- Modified flag set on commandExecuted
- Modified flag cleared on save
- loadWorkspaceIfExists succeeds with existing sidecar
- loadWorkspaceIfExists is no-op without sidecar
- revertToSaved restores pre-edit state
- 5+ unit tests

**Dependencies**: T1 (uses WorkspaceFile)

---

### T3 — MainWindow Save/Revert UI (Frontend, Size: M)

**Owner**: frontend

**Files to modify**:
- `src/lumen/ui/MainWindow.h` / `.cpp`
- `src/lumen/app/Application.h` / `.cpp` — own WorkspaceManager

**Implementation**:

File menu additions (after Open CSV, before Quit):
- "Save Workspace" (Ctrl+S) → calls workspaceManager_->saveWorkspace(currentDocPath_)
- "Save Workspace As..." → QFileDialog → saveWorkspaceAs
- "Revert to Saved" → workspaceManager_->revertToSaved, refresh plot
- Separator before Quit

Close/quit handler (closeEvent override update):
- If workspaceManager_->isModified(currentDocPath_):
  QMessageBox::question "Save workspace for <filename>?"
  Save → save then close. Discard → close without saving.
  Cancel → abort close.

Window title: append " ●" when modified, remove on save.
Connect workspaceManager_->modifiedChanged to update title.

Status bar: show "●" dot or "Saved" indicator.

Application.h/.cpp: create WorkspaceManager after CommandBus,
DocumentRegistry, PlotRegistry. Pass to MainWindow.

MainWindow stores `currentDocPath_` (set in loadFile handler).

**Acceptance criteria**:
- Save Workspace creates .lumen.json sidecar
- Save As picks custom path
- Revert restores saved state
- Close with unsaved edits shows prompt
- Window title shows modification indicator
- All existing tests still pass

**Dependencies**: T2 (needs WorkspaceManager API)

---

### T4 — QA Phase 4.1 Tests (QA, Size: M)

**Owner**: qa

**Files to create**:
- `tests/unit/test_workspace_file.cpp` (if not done by T1)
- `tests/unit/test_workspace_manager.cpp` (if not done by T2)
- `tests/integration/test_persistence_roundtrip.cpp`
- `tests/integration/test_sidecar_auto_load.cpp`
- `tests/integration/test_revert_to_saved.cpp`

**Integration tests**:
- Roundtrip: open CSV → edit line color → save → "close" (clear
  scene) → reopen → verify color restored
- Auto-load: create a .lumen.json fixture → open CSV → verify
  edits applied automatically
- Revert: edit → save → edit again → revert → verify state
  matches first save

**Phase 3a/3b regression**: verify all 217 existing tests still
pass.

**Acceptance criteria**:
- 6+ unit tests for WorkspaceFile
- 5+ unit tests for WorkspaceManager
- 3 integration tests
- All 217 pre-existing tests pass
- Target: ≥240 total tests

**Dependencies**: T1, T2, T3

---

### M4.1 — Human Verification Gate

**Not a task — a gate between sub-phases.**

Human must verify with real electrophysiology CSV:
1. Edit line color, axis labels, title
2. Ctrl+S saves workspace
3. .lumen.json appears next to CSV
4. Close and reopen → edits restored
5. Edit, don't save, close → prompted
6. Revert to Saved works

**If any fails**: fix before starting T5.

---

## Sub-phase 4.2 — Figure Export

### T5 — FigureExporter (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/core/io/FigureExporter.h`
- `src/lumen/core/io/FigureExporter.cpp`
- `tests/unit/test_figure_exporter.cpp`

**Implementation**:

```cpp
namespace lumen::core::io {

class FigureExporter {
public:
    enum class Format { Png, Svg, Pdf };

    struct Options {
        Format format = Format::Png;
        int widthPx = 1050;
        int heightPx = 700;
        int dpi = 300;
        bool transparentBackground = false;
        QString outputPath;
    };

    /// Returns empty string on success, error message on failure.
    static QString exportFigure(const plot::PlotScene* scene,
                                const Options& opts);
};

}
```

PNG: create QImage(widthPx, heightPx, Format_ARGB32), QPainter on
it, call PlotRenderer::render(painter, *scene, QSizeF(widthPx, heightPx)).
If transparentBackground, fill with Qt::transparent before render
(PlotRenderer draws background first, but we skip it — add a
`skipBackground` option or fill transparent after background).
Save with QImage::save at specified DPI.

SVG: QSvgGenerator with specified size, QPainter, same render call.
Requires Qt6::Svg module — add to CMakeLists.

PDF: QPdfWriter with specified size and DPI, QPainter, same render.
Requires Qt6::PrintSupport or QPdfWriter (available in Qt6::Gui).

**Acceptance criteria**:
- PNG produces file at specified pixel size
- SVG produces valid SVG (contains `<svg>` root)
- PDF produces valid PDF (starts with `%PDF-`)
- Transparent PNG has no opaque background
- Export does not modify PlotScene state
- Invalid path returns error message
- Empty scene exports without crash
- 8+ unit tests

**Dependencies**: M4.1 gate (persistence must work first)

---

### T6 — ExportDialog + MainWindow Integration (Frontend, Size: M)

**Owner**: frontend

**Files to create**:
- `src/lumen/ui/ExportDialog.h`
- `src/lumen/ui/ExportDialog.cpp`

**Files to modify**:
- `src/lumen/ui/MainWindow.h` / `.cpp` — add Export Figure menu
- `src/lumen/CMakeLists.txt` — add ExportDialog sources

**ExportDialog** (non-modal QDialog, Apple-mood styled):
- Format: QButtonGroup with radio buttons (PNG/SVG/PDF)
- Size presets: QComboBox ("As shown", "Publication single column
  3.5in", "Publication double column 7in", "Custom")
- Custom: width/height QSpinBox (visible when Custom selected)
- DPI: QComboBox (72/150/300/600) — disabled for SVG
- Background: QComboBox (White/Transparent) — disabled for SVG
- File path: QLineEdit + Browse button (QFileDialog::getSaveFileName)
- Export button + Cancel button

On Export: construct FigureExporter::Options, call exportFigure.
On success: accept dialog, MainWindow shows status bar message
"Exported to <path>" for 3 seconds. On failure: QMessageBox with
error.

MainWindow: File → "Export Figure..." (Ctrl+E), opens ExportDialog
with current PlotScene.

**Acceptance criteria**:
- Dialog opens with sensible defaults
- Format selection disables irrelevant controls
- Size presets compute correct pixel dimensions
- Export produces file at chosen location
- Status bar success message appears
- Export does not change plot state

**Dependencies**: T5 (FigureExporter must exist)

---

### T7 — QA Phase 4.2 Tests (QA, Size: M)

**Owner**: qa

**Files to create**:
- `tests/unit/test_figure_exporter.cpp` (if not done by T5)
- `tests/integration/test_export_png_roundtrip.cpp`
- `tests/integration/test_export_svg_contains_series.cpp`
- `tests/integration/test_export_preserves_edits.cpp`

**Regression**: all 217 + Phase 4.1 tests still pass.

**Acceptance criteria**:
- 8+ FigureExporter unit tests
- 3 integration tests
- All prior tests pass
- Target: ≥260 total tests

**Dependencies**: T5, T6

---

### M4.2 — Human Verification Gate

Human must verify:
1. Export PNG at 300 DPI → file correct size, looks right
2. Export SVG → opens in browser, vector rendering
3. Export PDF → opens in viewer, plot renders
4. Edits visible in exported files
5. Export does not change plot state

---

## Closing

### T8 — Docs Closing (Docs, Size: S) — REVIEW AND STATUS IN SAME COMMIT

**Owner**: docs (or architect/coordinator)

**Files to create/modify IN ONE COMMIT**:
- `docs/reviews/phase-4-review.md` — full review
- `.lumen-ops/STATUS.md` — closing entry
- `README.md` — update status to Phase 4, add Save/Export to
  "What Lumen can do"
- `src/lumen/core/io/CLAUDE.md` — new submodule doc

**HARD RULE**: `git add` all four files and `git commit` them
together. This is one commit, one task, non-negotiable.

After commit: `git tag vphase-4 && git push origin main --tags`.

**Acceptance criteria**:
- Review document exists in same commit as STATUS close
- README reflects Phase 4 capabilities
- Tag pushed

**Dependencies**: M4.2 gate

---

## Parallel Execution Schedule

```
Sub-phase 4.1:
  Round 1 (parallel, ~4h):
    Backend: T1 (WorkspaceFile) + T2 (WorkspaceManager)

  Round 2 (~3h):
    Frontend: T3 (MainWindow save/revert UI)

  Round 3 (~2h):
    QA: T4 (persistence tests)

  Gate M4.1: Human verifies roundtrip

Sub-phase 4.2:
  Round 4 (parallel, ~3h):
    Backend: T5 (FigureExporter)
    Frontend: T6 (ExportDialog)

  Round 5 (~2h):
    QA: T7 (export tests)

  Gate M4.2: Human verifies export

Closing:
  Round 6 (~30min):
    Docs: T8 (review+STATUS+README in ONE commit, then tag)
```

**Total wall time**: ~14-16 hours across rounds.

## Risks

- **Qt6::Svg availability**: Ubuntu 24.04 apt may not include
  qt6-svg-dev. Check and install if needed before T5.
- **QPdfWriter availability**: QPdfWriter is in Qt6::Gui (not a
  separate module). Should be available. Verify.
- **Sidecar file permissions**: if CSV is on a read-only
  filesystem, sidecar save fails. Show error message.
- **CommandBus modified tracking**: all plot mutations must go
  through commands. Backend audit in T2 to confirm no direct
  mutations bypass CommandBus.

## Lessons Applied

- **Review in same commit as STATUS** (Phase 3a/3b lesson).
- **QA exclusively owns test files** (Phase 1 lesson).
- **Human verification gate between sub-phases** (new for Phase 4).
- **Coordinator commits on behalf of permission-blocked agents**.
- **Build verification with QT_QPA_PLATFORM=offscreen and
  ASAN_OPTIONS=detect_leaks=0 before every commit**.
