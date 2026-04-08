# Phase 1 Review — Data Layer and First UI Shell

**Date**: 2026-04-09
**Reviewer**: Architect agent
**Spec**: `docs/specs/phase-1-spec.md`
**Plan**: `docs/plans/phase-1-plan.md`

---

## What shipped

Phase 1 delivered the complete data-loading pipeline: from a CSV file
on disk to an interactive table view inside the Lumen window, styled
with the Apple-mood design system and the Inter typeface.

### Components delivered

1. **CsvReader** — RFC 4180 streaming parser with NaN handling, type
   inference, header detection, BOM-aware UTF-8, configurable delimiter,
   and error reporting with line/column numbers.

2. **DataFrame / Column / ColumnType** — Move-only in-memory column
   store. Type-erased columns (Int64, Double, String). Bounds-checked
   access. shared_ptr ownership for cross-thread transport.

3. **EventBus** — QObject-based publish/subscribe with typed event
   enum. Thread-safe publish via mutex + snapshot dispatch. Auto-cleanup
   on receiver destruction.

4. **DocumentRegistry** — Owns open DataFrames keyed by file path.
   Duplicate detection. Open/close signals integrated with EventBus.

5. **FileLoader** — Async CSV loading on a QThread. Progress signals,
   cancellation via atomic flag, result delivery via queued connection.

6. **DataTableDock / DataFrameTableModel** — Read-only QTableView in a
   QDockWidget. 6-significant-digit formatting, NaN in grey, column
   sorting via QSortFilterProxyModel, alternating row colors.

7. **DesignTokens / StyleManager** — C++ constexpr tokens matching
   `docs/design/design-system.md`. QSS light theme. Inter font loaded
   and embedded via qt_add_resources.

8. **File-open UI flow** — File → Open CSV (Ctrl+O), QFileDialog,
   async loading with status bar feedback, error dialogs, recent files
   (last 10, QSettings-backed).

9. **Test fixtures** — 9 hand-crafted CSVs plus a Python generator for
   100K-row performance fixtures.

10. **Integration tests** — 5 end-to-end tests covering the full path
    from file to DocumentRegistry, including NaN and error handling.

---

## What works

Verified by the project owner (T.Y.) with real electrophysiology data:

- Open `wt0_cap100nM_d20251029_s002_before_1x_raw.csv` (3499 rows × 9
  columns, contains NaN at derivative boundary rows)
- Data displays correctly in DataTableDock
- NaN values rendered as "NaN" in text.tertiary grey (#8E8E93)
- Column headers match CSV headers
- Click column header to sort ascending/descending
- Status bar shows "Loaded filename (3499 rows × 9 cols)"
- Recent files menu remembers last opened file
- Ctrl+Q quits cleanly with no ASan/UBSan reports
- Inter font renders in all UI text
- Apple-mood light theme applied (rounded elements, soft colors)

**Test results**: 72/72 pass (67 unit + 5 integration), ASan + UBSan
clean, zero compiler warnings.

---

## What was deferred

| Item | Reason | Target |
|------|--------|--------|
| T11 Docs update (README, CLAUDE.md sync) | Low priority, no blocker | Phase 2 start |
| Dark theme | Design system has tokens; QSS not written | Phase 3 |
| DateTime type inference | Not needed for current data | Phase 3 |
| CP949 / non-UTF-8 encoding | Owner's data is UTF-8 | Phase 3 |
| Multiple file tabs | Single document sufficient for now | Phase 3 |
| Data editing / mutation | Read-only in Phase 1 | Phase 4 |
| CommandBus (undo/redo) | No mutations to undo yet | Phase 3 |

---

## Lessons learned

### 1. Fixture file ownership must be exclusive
Both Backend and QA agents created overlapping CSV fixtures, causing
merge conflicts on 4 files. **Rule for Phase 2+**: QA exclusively
owns `tests/fixtures/`. Backend uses inline test data or imports from
QA's fixtures. No exceptions.

### 2. Subagent sandbox limits require coordinator overhead
Agents could not run `git commit`, `curl`, or `wget`. The coordinator
committed on their behalf and downloaded the Inter font. This is
acceptable but adds ~5 minutes per round. For Phase 2, consider
pre-downloading resources before launching agents.

### 3. Qt 6.4 is sufficient (for now)
Ubuntu 24.04 apt provides Qt 6.4.2. The original spec said 6.6+.
Phase 1 encountered zero API gaps at 6.4. Phase 2's QPainter work
should also be fine, but monitor for any 6.5+ APIs needed.

### 4. shared_ptr for cross-thread DataFrame is pragmatic
Qt signals cannot transport move-only types. Using shared_ptr instead
of unique_ptr for the FileLoader→DocumentRegistry handoff is a
justified deviation from the "unique_ptr everywhere" rule (CLAUDE.md
rule 1). Document this pattern for future agents.

### 5. Three-round parallel execution works well
The dependency graph in phase-1-plan.md accurately predicted the
critical path (T1→T2→T3→T4→T5→T6) and parallel tracks (T7, T8, T9,
T10). All three rounds completed without blocking. This model should
be repeated for Phase 2.

---

## Exit checklist

- [x] All spec deliverables done (D1–D7)
- [x] 10/11 plan tasks complete (T11 deferred, non-blocking)
- [x] 72/72 tests pass, ASan+UBSan clean
- [x] Owner verified with real data: "Opened successfully"
- [x] ADRs 009–011 written and merged
- [x] Architecture doc updated with Phase 1 additions
- [x] STATUS.md updated with closing entry
- [x] This review written
- [ ] `docs/specs/phase-2-spec.md` drafted (next step)
- [ ] T11 Docs update (deferred to Phase 2 start)
