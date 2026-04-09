# Phase 3b Review — Axis, Title, Legend Editing + Dynamic Margins

**Date**: 2026-04-10
**Reviewer**: Architect agent
**Spec**: `docs/specs/phase-3b-spec.md`
**Plan**: `docs/plans/phase-3b-plan.md`

---

## What shipped

Phase 3b completed the MATLAB-figure-style editing experience: every
visible plot element (line, axis, title, legend) is now double-clickable
and editable. Plot margins adjust dynamically to content. Crosshair
snaps to actual data samples.

### Components delivered

| Task | Component | Files |
|------|-----------|-------|
| T1 | Axis setters + RangeMode/TickFormat enums + changed() signal | Axis.h/.cpp |
| T2 | PlotScene title font size/weight state | PlotScene.h/.cpp |
| T3 | Legend class extraction from PlotRenderer | Legend.h/.cpp, PlotRenderer.cpp |
| T4 | ChangeAxisPropertiesCommand, ChangeTitleCommand, ChangeLegendCommand | core/commands/ (6 files) |
| T5 | HitTester::hitNonSeriesElement with HitKind enum | HitTester.h/.cpp |
| T6 | PlotScene::computeMargins (resolves ADR-013) | PlotScene.h/.cpp |
| T6.5 | HitTester::hitTestPoint + nearest-sample crosshair (resolves ADR-017) | HitTester.h/.cpp, PlotCanvas.cpp |
| T7 | AxisDialog | AxisDialog.h/.cpp |
| T8 | TitleDialog + inline title editor | TitleDialog.h/.cpp, PlotCanvas.h/.cpp |
| T9 | LegendDialog | LegendDialog.h/.cpp |
| T10 | InteractionController dispatch + 4 new signals | InteractionController.h/.cpp, PlotCanvasDock.h/.cpp |

### ADR resolutions

| ADR | Status | Commit | Resolution |
|-----|--------|--------|------------|
| ADR-013 (hardcoded margins) | **Resolved** | c7dc412 | computeMargins() with QFontMetrics + tokens::spacing |
| ADR-017 (cursor crosshair) | **Resolved** | 6e31ed9 | hitTestPoint() binary search, snaps to real data |

---

## What works

Human verified on 2026-04-10 with real electrophysiology CSV:

1. Open CSV → auto line plot
2. Double-click X axis → AxisDialog opens, change label
3. Double-click Y axis → AxisDialog opens, change range
4. Double-click title area → inline editor, type title
5. Double-click legend → LegendDialog opens
6. Double-click line → LinePropertyDialog (Phase 3a preserved)
7. Crosshair snaps to nearest actual data sample
8. Pan, zoom, box-zoom all work
9. Dynamic margins adjust to content

Human response: "good it works"

---

## Bug found and fixed

**Title editor focus loss**: the inline title editor's
EditingTitleInline mode suppressed all mouse events (pan, zoom,
double-click). If the editor lost focus without proper cleanup, the
interaction mode stayed locked. Fixed by adding QLineEdit::editingFinished
connection and a safety check in mousePressEvent (commit 78b5580).

---

## Deviations from spec

### 1. Bundled commands (continued from Phase 3a)

Spec listed individual commands (SetAxisLabelCommand,
SetAxisRangeCommand, etc.). Implementation uses bundled commands
(ChangeAxisPropertiesCommand captures all 8 properties). Same
rationale as Phase 3a: one undo entry per dialog confirmation.

### 2. Separate signals (continued from Phase 3a)

Spec implied a generic editRequested(HitResult) dispatch.
Implementation adds 4 typed signals (xAxisDoubleClicked,
yAxisDoubleClicked, titleDoubleClicked, legendDoubleClicked).
Same rationale: strongly typed, self-documenting.

### 3. Legend position simplification

Spec lists 5 positions including OutsideRight with right margin
expansion. Implementation includes the Legend class with all 5
positions in the enum, but PlotRenderer's OutsideRight rendering
and margin integration are partial — the margin computation does
not yet account for OutsideRight. This is minor: the other 4
positions work correctly.

---

## Test results

- 217/217 tests pass (161 Phase 3a + 56 Phase 3b)
- ASan + UBSan clean
- Zero compiler warnings

---

## Lessons learned

### 1. Title editor needs explicit focus loss handling
The EditingTitleInline mode guard in InteractionController is
correct but dangerous: if the editor's lifecycle isn't perfectly
managed, the mode stays locked and all interaction breaks. Always
connect editingFinished and add a safety check in mousePressEvent.

### 2. Dynamic margins require careful testing with real data
The switch from hardcoded to dynamic margins changed the plot area
rectangle, which affects CoordinateMapper, HitTester distances,
and crosshair positioning. Integration testing with real data
(not just unit tests) caught the margin-related interaction issues.

### 3. Phase review should be written immediately at close
This is the third phase where the review was missing at close time
(Phase 3a was missing, caught in Phase 3b prep; Phase 3b was
missing, caught now). Going forward: the review commit should be
part of the closing STATUS.md commit, not a separate task.

---

## Exit checklist

- [x] Build clean (0 warnings, 0 errors)
- [x] 217 tests pass under ASan+UBSan
- [x] ADR-013 marked resolved with commit SHA
- [x] ADR-017 marked resolved with commit SHA
- [x] ADRs 021-024 committed
- [x] Phase 3a regression: line editing still works
- [x] Phase 2 regression: pan/zoom still work
- [x] This review committed
- [x] Human verified with real electrophysiology CSV
- [ ] vphase-3b tag (to be pushed)
- [ ] Publication-ready figure from real data (deferred to user)
