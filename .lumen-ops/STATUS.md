# Lumen — Status Log

Append-only. One entry per session per agent.

## 2026-04-09 — bootstrap
Repository initialized. Phase 0 in progress.

## 2026-04-08 — Architect session 1 (Phase 1 planning)
- Wrote `docs/specs/phase-1-spec.md`: CSV parser, DataFrame, EventBus,
  DocumentRegistry, FileLoader, DataTableDock, Apple-mood QSS v1, Inter font.
- Wrote `docs/plans/phase-1-plan.md`: 11 tasks, critical path identified
  (T1→T2→T3→T4→T5→T6), parallel tracks for QA fixtures, Frontend styling.
- Drafted ADRs 009 (threading), 010 (EventBus vs signals), 011 (Inter font).
- Updated `docs/architecture.md` with Phase 1 data-loading flow and threading.
- Awaiting human review and approval before commit.

## 2026-04-09 — Phase 1 close

### Delivered tasks (from phase-1-plan.md)

| Task | Owner | Status | Commit |
|------|-------|--------|--------|
| T1 CsvReader | Backend | Done | 5e4f601 |
| T2 DataFrame + Column | Backend | Done | 5e4f601 |
| T3 EventBus + DocumentRegistry | Backend | Done | 0e5aa02 |
| T4 FileLoader (async QThread) | Backend | Done | 0e5aa02 |
| T5 File-open UI flow | Frontend | Done | 3033876 |
| T6 Integration tests (end-to-end) | QA | Done | 7ced3a6 |
| T7 Test fixtures (9 CSVs + generator) | QA | Done | fa8f6ce |
| T8 Apple-mood QSS v1 + DesignTokens | Frontend | Done | 8dea97e |
| T9 Inter font integration (4 weights, OFL) | Frontend | Done | 8dea97e |
| T10 DataTableDock + DataFrameTableModel | Frontend | Done | 8a1f37e |
| T11 Docs update (README, CLAUDE.md sync) | Docs | Deferred | — |

**10 of 11 tasks completed. T11 deferred to Phase 2 start.**

### Test results
- 72/72 tests pass (67 unit + 5 integration)
- ASan + UBSan clean in Debug mode
- Zero compiler warnings under -Wall -Wextra -Wpedantic -Werror

### Real-data verification
- Owner (T.Y.) opened electrophysiology patch-clamp CSV
  (wt0_cap100nM_d20251029_s002_before_1x_raw.csv: 3499 rows × 9 cols)
- Data displayed correctly in DataTableDock
- NaN values rendered in grey (text.tertiary)
- Column sorting, status bar, recent files all functional
- Confirmed: "Opened successfully"

### Agents involved
- **Architect**: Phase 1 spec, plan, ADRs 009–011, architecture update
- **Backend**: T1, T2, T3, T4 (CsvReader → DataFrame → EventBus/Registry → FileLoader)
- **Frontend**: T8, T9, T10, T5 (DesignTokens → Inter font → DataTableDock → file-open UI)
- **QA**: T7, T6 (fixtures → integration tests)
- **Integration**: merge conflict resolution handled by coordinator
- **Docs**: not activated (T11 deferred)

### Execution model
Three parallel rounds, each with 2–3 agents working simultaneously:
- Round 1: Backend(T1+T2) + Frontend(T8+T9) + QA(T7)
- Round 2: Backend(T3+T4) + Frontend(T10)
- Round 3: Frontend(T5) + QA(T6)
Merge conflicts resolved after each round before launching the next.

### Lessons learned
1. **Fixture file collisions**: Both Backend and QA created overlapping
   fixture files (simple_3x4.csv, with_nan.csv, etc.). The merge
   produced conflicts. Fix: assign fixture creation to QA exclusively
   (per plan), and have Backend use inline test data or wait for QA.
2. **Agent sandbox limits**: Subagents could not run git commands or
   network downloads (curl/wget). The coordinator had to commit and
   download fonts on their behalf. Acceptable but adds overhead.
3. **Qt 6.4 vs 6.6 gap**: Ubuntu apt provides Qt 6.4.2; the original
   spec said 6.6+. Lowered the CMake requirement to 6.4 in Phase 0.
   No API issues encountered in Phase 1. Monitor for Phase 2 (QPainter
   features).
4. **shared_ptr for cross-thread DataFrame**: Qt signals cannot transport
   move-only types. Backend used std::shared_ptr<DataFrame> instead of
   unique_ptr for the FileLoader→DocumentRegistry handoff. This is a
   pragmatic deviation from the "unique_ptr everywhere" rule, documented
   in the code.
5. **Parallel rounds work**: The 3-round parallel model (Backend critical
   path + Frontend parallel track + QA fixtures) completed Phase 1 in a
   single session. The dependency graph in phase-1-plan.md was accurate.

## 2026-04-09 — Architect session 2 (Phase 2 planning)
- Wrote `docs/specs/phase-2-spec.md`: plot engine (line plot), 7 deliverables.
- Wrote `docs/plans/phase-2-plan.md`: 10 tasks, critical path
  (T1+T2+T3 parallel → T4 → T5 → T6 → T7), 4-week schedule.
- Applied Phase 1 lessons: QA-exclusive fixtures, early CMake module,
  shared_ptr convention documented.
- Awaiting human review and approval.

## 2026-04-09 — Phase 2 implementation complete
- Backend (3 rounds): T9 CMake, T1 NiceNumbers+CoordMapper,
  T2 ViewTransform, T3 LineSeries, T4 Axis+PlotScene.
- Frontend (3 rounds): T5 PlotRenderer, T6 PlotCanvas+Interaction,
  T7 PlotCanvasDock+column picker+auto-plot.
- Human verified 7 interaction checks with real electrophysiology CSV.
- 121 tests passing (72 Phase 1 + 36 Phase 2 plot + 3 renderer + 5 integration + 5 misc).
- Agent permission issues: T4, T5, T6+T7 implemented by coordinator
  after subagents were blocked by sandbox restrictions.

## 2026-04-09 — Phase 2.5 opening (Architect session 3)
- Wrote `docs/specs/phase-2.5-spec.md`: consolidation round, 6 deliverables.
- Wrote `docs/plans/phase-2.5-plan.md`: 6 tasks, 3 rounds + human re-verification.
- Wrote ADRs 013-017 documenting Phase 2 decisions (margins, precision,
  NiceNumbers, inline interaction, crosshair simplification).
- Wrote `docs/reviews/phase-2-review.md` with human verification evidence.
- Updated `docs/architecture.md` with Phase 2 rendering pipeline.
- Remaining: T1 PlotRegistry (Backend), T2 plot/ tests (QA), T6 README (Docs).

## 2026-04-09 — Phase 2.5 close

### Delivered
- T1 PlotRegistry + EventBus 4 plot events (Backend)
- T2 18 plot module unit tests (QA)
- T3 ADRs 013-017 (Architect)
- T4 Phase 2 review (Architect)
- T5 architecture.md Phase 2 section (Architect)

### Test results
- 146/146 tests pass (139 unit + 7 integration), ASan+UBSan clean

### Human re-verification (7 interaction checks)
All passed on 2026-04-09:
1. Open CSV → auto line plot
2. Pan via left-drag
3. Wheel zoom (Shift=X, Ctrl=Y)
4. Right-drag box zoom
5. Double-click reset
6. Crosshair tooltip
7. Column picker updates plot

### Exit checklist
- [x] Build clean (0 warnings)
- [x] ≥90 tests green (146)
- [x] 5 ADRs committed (013-017)
- [x] Phase 2 review committed
- [x] architecture.md updated
- [x] Human re-verification pass
- [ ] README.md update (T6, deferred to Phase 3 start)

## 2026-04-09 — Phase 3a opening (Architect session 4)
- Wrote `docs/specs/phase-3a-spec.md`: line property editing, 6 deliverables
  (LinePropertyDialog, HitTester, double-click-to-edit, visibility toggle,
  style persistence, tests).
- Wrote `docs/plans/phase-3a-plan.md`: 7 tasks, critical path
  T1+T2 parallel → T3 → T4 → T5. CommandBus (ADR-018), HitTester
  (ADR-019), InteractionController (ADR-020) as architectural additions.
- Drafted ADRs 018-020: CommandBus design, HitTester extraction
  (resolves ADR-017), InteractionController FSM (resolves ADR-016).
- Updated `docs/architecture.md` with Phase 3a section: CommandBus
  data flow, HitTester, InteractionController, LineSeries mutability,
  style persistence, resolved/remaining tech debt.
- Awaiting human review and approval before commit.

## 2026-04-09 — Phase 3a close addendum
- Wrote `docs/reviews/phase-3a-review.md` (was missing from Phase 3a exit).
- Documented three intentional deviations from spec: bundled commands,
  separate signals, HitTester series-only scope.
- ADR-016 marked resolved in Phase 3a (InteractionController extraction).
- ADR-017 marked as to-be-resolved in Phase 3b (hitTestPoint + crosshair).

## 2026-04-09 — Phase 3b opening (Architect session 5)
- Wrote `docs/plans/phase-3b-plan.md`: 15 tasks, 7 rounds.
  - Resolves ADR-013 (dynamic margins, T6)
  - Resolves ADR-017 (nearest-sample crosshair, T6.5)
  - 3 bundled commands (ChangeAxisProperties, ChangeTitle, ChangeLegend)
  - 3 new dialogs (AxisDialog, TitleDialog+inline editor, LegendDialog)
  - Legend class extracted from PlotRenderer
  - HitTester extended with hitNonSeriesElement() and hitTestPoint()
  - Target ≥200 tests
- Drafted ADRs 021-024: non-modal dialogs, dynamic margins (resolves
  ADR-013), inline title editor, HitTester precedence ordering.
- Updated `docs/architecture.md` with Phase 3b section.
- Updated ADR-013, ADR-016, ADR-017 status notes.
- Awaiting human review and approval.

## 2026-04-10 — Phase 3b close

### Delivered
- T1+T2+T3 Axis setters+signal, PlotScene title state, Legend class (Backend)
- T4 ChangeAxisPropertiesCommand, ChangeTitleCommand, ChangeLegendCommand (Backend)
- T5 HitTester hitNonSeriesElement with HitKind enum (Frontend)
- T6 PlotScene::computeMargins replacing hardcoded 60/50/30/15 (Frontend, resolves ADR-013)
- T6.5 HitTester::hitTestPoint + nearest-sample crosshair (Frontend, resolves ADR-017)
- T7+T8+T9+T10 AxisDialog, TitleDialog+inline editor, LegendDialog, dispatch (Frontend)
- ADRs 021-024 (Architect)
- Phase 3a review retroactively written (Architect)
- Fix: title editor focus loss handler preventing stuck interaction mode

### ADR resolutions
- ADR-013 (hardcoded margins): RESOLVED by T6 computeMargins
- ADR-017 (cursor crosshair): RESOLVED by T6.5 hitTestPoint

### Test results
- 217/217 tests pass, ASan+UBSan clean

### Human verification
All items confirmed working on 2026-04-10:
1. Open CSV → auto line plot
2. Double-click X axis → AxisDialog, change label
3. Double-click Y axis → AxisDialog, change range
4. Double-click title area → inline editor
5. Double-click legend → LegendDialog
6. Double-click line → LinePropertyDialog (Phase 3a preserved)
7. Crosshair snaps to nearest data sample
8. Pan, zoom, box-zoom work
9. Dynamic margins adjust to content

Human response: "good it works"
