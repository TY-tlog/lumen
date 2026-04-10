# Phase 4 Review — Save and Export

**Date**: 2026-04-11
**Reviewer**: Architect agent
**Spec**: `docs/specs/phase-4-spec.md`
**Plan**: `docs/plans/phase-4-plan.md`

---

## What shipped

Phase 4 delivered edit persistence (workspace sidecar files) and
figure export (PNG/SVG/PDF), transforming Lumen from a viewer-with-
editing into an end-to-end tool.

### Sub-phase 4.1 — Edit Persistence

| Task | Component | Description |
|------|-----------|-------------|
| T1 | WorkspaceFile | JSON v1 sidecar (.lumen.json) serialization/deserialization |
| T2 | WorkspaceManager | Modified tracking via CommandBus, auto-load on documentOpened, save/revert API |
| T3 | MainWindow UI | Save Workspace (Ctrl+S), Save As, Revert to Saved, unsaved-changes prompt, "●" indicator |

### Sub-phase 4.2 — Figure Export

| Task | Component | Description |
|------|-----------|-------------|
| T5 | FigureExporter | PNG (QImage), SVG (QSvgGenerator), PDF (QPdfWriter), all via PlotRenderer (ADR-026) |
| T6 | ExportDialog | Format/size preset/DPI/background controls, file path picker |

---

## Human verification

### M4.1 gate (edit persistence) — passed 2026-04-11
1. Opened real electrophysiology CSV
2. Edited line color, axis labels, title
3. Ctrl+S saved workspace → .lumen.json appeared next to CSV
4. Closed and reopened → all edits restored
5. Edited without saving → close prompted Save/Discard/Cancel
6. Discarded → edits gone on reopen, earlier saves preserved

Human response: "yes."

### M4.2 gate (figure export) — passed 2026-04-11
1. Export Figure (Ctrl+E) → ExportDialog opened
2. PNG at 300 DPI, Publication single column → file created, correct size
3. SVG export → vector rendering in browser
4. PDF export → renders in PDF viewer

Human response: "yes"

---

## Test results

- 247/247 tests pass (217 Phase 3b + 19 workspace + 11 export)
- ASan + UBSan clean
- Zero compiler warnings

---

## ADRs delivered

| ADR | Decision |
|-----|----------|
| ADR-025 | .lumen.json sidecar format v1 (JSON, portable, human-readable) |
| ADR-026 | Figure export reuses PlotRenderer (single rendering code path) |
| ADR-027 | Synchronous export in main thread (revisit if >500ms) |

---

## Lessons learned

### 1. Two sub-phases with human gates works well
The M4.1 gate before starting export ensured persistence was solid
before building on it. This prevented wasted work if persistence
had failed.

### 2. FigureExporter stub pattern for parallel development
Frontend created a FigureExporter stub so ExportDialog could
compile while Backend implemented the real version in parallel.
Merge resolved by keeping Backend's real implementation. This
pattern should be reused when Frontend and Backend need the same
header.

### 3. Review in same commit as STATUS — enforced
This review is being committed in the same commit as the closing
STATUS entry, applying the lesson from Phase 3a/3b where reviews
were repeatedly forgotten.

---

## Exit checklist

- [x] WorkspaceFile load/save roundtrip passes
- [x] WorkspaceManager modified-tracking works
- [x] Save Workspace menu item functional
- [x] Auto-load sidecar on document open
- [x] Revert to Saved functional
- [x] Unsaved-changes prompt on close
- [x] FigureExporter PNG/SVG/PDF all work
- [x] ExportDialog functional
- [x] Export does not mutate PlotScene
- [x] Build clean
- [x] 247 tests pass under ASan+UBSan
- [x] ADR-025, 026, 027 committed
- [x] This review in SAME commit as STATUS close
- [x] Human verified persistence roundtrip (M4.1)
- [x] Human verified export in 3 formats (M4.2)
- [x] vphase-4 tag (this commit)
