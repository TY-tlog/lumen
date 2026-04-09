# Phase 3a Review — Line Property Editing

**Date**: 2026-04-09
**Reviewer**: Architect agent
**Spec**: `docs/specs/phase-3a-spec.md`
**Plan**: `docs/plans/phase-3a-plan.md`

---

## What shipped

Phase 3a delivered line series property editing via double-click,
backed by a CommandBus with undo/redo, and supported by HitTester
for click detection and InteractionController for clean event
handling.

### Components delivered

| Component | Files | Description |
|-----------|-------|-------------|
| CommandBus | core/CommandBus.h/.cpp, core/Command.h | Abstract Command base + undo/redo stack manager |
| ChangeLineStyleCommand | core/commands/ChangeLineStyleCommand.h/.cpp | Bundled command: captures style+name+visibility together |
| LineSeries mutability | plot/LineSeries.h/.cpp | setStyle(), setName(), setVisible(), isVisible() |
| PlotScene mutable access | plot/PlotScene.h/.cpp | seriesAt(index) returns non-const reference |
| PlotRenderer visibility | plot/PlotRenderer.cpp | Skips invisible series, greys legend for hidden |
| HitTester | plot/HitTester.h/.cpp | Static hitTest() for series-level pixel-distance detection |
| InteractionController | ui/InteractionController.h/.cpp | FSM (Idle/Panning/ZoomBoxing), all mouse logic extracted |
| PlotCanvas refactor | ui/PlotCanvas.h/.cpp | Thin rendering host, delegates to InteractionController |
| LinePropertyDialog | ui/LinePropertyDialog.h/.cpp | QDialog: color, width, style, name, visibility |
| Style persistence | ui/PlotCanvasDock.h/.cpp | QHash maps column names to custom styles/visibility/names |
| Application wiring | app/Application.h/.cpp | Owns CommandBus, passes to MainWindow |

---

## What works

Human verified all 9 acceptance checks on 2026-04-09:

1. Open CSV → auto line plot
2. Double-click ON line → LinePropertyDialog opens
3. Change color/width → OK → line updates
4. Double-click EMPTY space → view resets
5. Add second Y series, edit its style independently
6. Uncheck Visible → series disappears
7. Switch column → switch back → custom style preserved
8. Pan, zoom, crosshair still work
9. Undo works via CommandBus

Human response: "Yes perfectly works."

---

## Deviations from spec

Phase 3a's implementation made three intentional simplifications
relative to docs/specs/phase-3a-spec.md. These are design choices,
not failures — the implementation is correct and human-verified.

### 1. Bundled command instead of individual SetXxx commands

**Spec expected**: Six separate command classes (SetLineColorCommand,
SetLineWidthCommand, SetLineStyleCommand, SetLineNameCommand,
SetLineVisibleCommand, etc.).

**Implementation**: One `ChangeLineStyleCommand` that captures the
complete old and new state (PlotStyle + name + visibility) as a
single undo unit.

**Rationale**: The dialog collects all changes at once and applies
them on OK. Creating six commands per dialog interaction would
produce six undo entries for one logical edit. The bundled approach
gives one undo entry per dialog confirmation, which matches user
expectations.

**Impact on Phase 3b**: Phase 3b should follow the same bundled
pattern: `ChangeAxisPropertiesCommand`, `ChangeTitleCommand`,
`ChangeLegendCommand`.

### 2. Separate signals instead of generic editRequested

**Spec expected** (implicit in ADR-020): A generic
`editRequested(HitResult)` signal that PlotCanvasDock dispatches
based on HitKind.

**Implementation**: Two separate signals:
`seriesDoubleClicked(int seriesIndex)` and
`emptyAreaDoubleClicked()`.

**Rationale**: Phase 3a only has two double-click outcomes (series
hit or empty area). A generic signal adds indirection without
benefit. Each signal is strongly typed and self-documenting.

**Impact on Phase 3b**: Phase 3b adds four more signals
(xAxisDoubleClicked, yAxisDoubleClicked, titleDoubleClicked,
legendDoubleClicked) following the same pattern. If the signal
count exceeds 7-8 in Phase 4+, consolidate into a generic signal.

### 3. HitTester scope: series only, no HitKind enum

**Spec expected** (per ADR-019): HitTester with HitKind enum
supporting LineSeries, XAxis, YAxis, Title, Legend, PlotArea.

**Implementation**: HitTester::hitTest() returns
`optional<HitResult{seriesIndex, pixelDistance}>` for LineSeries
hits only. No HitKind enum.

**Rationale**: Phase 3a only needs series-level hit detection.
Building the full HitKind system for one use case would be
premature. The static method interface is clean and easily
extensible.

**Impact on Phase 3b**: Phase 3b extends HitTester with a new
method (e.g., hitAtRegion()) that returns axis/title/legend hits
via a HitKind enum, without modifying the existing hitTest()
interface. This follows the open/closed principle.

---

## ADR resolution status

| ADR | Phase 3a status | Explanation |
|-----|----------------|-------------|
| ADR-006 (CommandBus mandate) | Implemented | CommandBus + ChangeLineStyleCommand shipped |
| ADR-016 (interaction inline) | **Resolved** | InteractionController extracted from PlotCanvas (ADR-020) |
| ADR-017 (crosshair cursor) | **Foundation built, to be resolved in Phase 3b** | HitTester (ADR-019) provides series-level detection. Crosshair still shows cursor position. Phase 3b adds hitTestPoint() for nearest-sample snap (Task T6.5). |
| ADR-018 (CommandBus design) | Implemented | As specified |
| ADR-019 (HitTester extraction) | Implemented | Series-level; Phase 3b extends with hitNonSeriesElement() and hitTestPoint() |
| ADR-020 (InteractionController) | Implemented | As specified |

---

## Test results

- 161/161 tests pass (146 Phase 2.5 + 9 CommandBus + 6 HitTester)
- ASan + UBSan clean (except known fontconfig LeakSanitizer
  false positive)
- Zero compiler warnings under -Wall -Wextra -Wpedantic -Werror

---

## Lessons learned

### 1. Bundled commands match dialog UX better than granular commands
The dialog-confirmation model (edit multiple properties → OK/Cancel)
maps naturally to one command per dialog. Phase 3b should continue
this pattern. Reserve granular commands for programmatic/scripted
edits (Phase 7+).

### 2. Subagent permission issues persist but are manageable
All four implementation tasks (T1 Backend, T2 Frontend, T4+T5
Frontend) required coordinator intervention for git commits. Two
tasks (T1 Backend, T2 Frontend) completed their code but couldn't
commit. The coordinator committed on their behalf. Budget ~5 min
per agent for commit overhead.

### 3. Merge conflict resolution requires careful file tracking
Round 1 merge (Backend T1+T3 and Frontend T2 in parallel) produced
conflicts in 9 files. Conflict resolution required manually copying
files from the frontend worktree after the merge kept backend's
versions. Future: merge one branch at a time, build-verify between
merges.

---

## Exit checklist

- [x] D1 LinePropertyDialog shipped
- [x] D2 HitTester shipped (series-level)
- [x] D3 Double-click to edit with disambiguation
- [x] D4 Series visibility toggle
- [x] D5 Style persistence across column changes
- [x] D6 Unit tests (15 new: 9 CommandBus + 6 HitTester)
- [x] Human verified with real electrophysiology CSV
- [x] 161/161 tests pass, ASan+UBSan clean
- [x] ADRs 018-020 committed
- [ ] Phase 3a review (this document — was missing, now written)
- [ ] ADR-016 status update (Phase 3b Task A2)
