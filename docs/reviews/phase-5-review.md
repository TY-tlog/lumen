# Phase 5 Review — Scatter and Bar Plots with PlotItem Abstraction

**Date**: 2026-04-11
**Reviewer**: Architect agent
**Spec**: `docs/specs/phase-5-spec.md`
**Plan**: `docs/plans/phase-5-plan.md`

---

## What shipped

Phase 5 introduced scatter and bar plot types alongside lines,
backed by a PlotItem abstraction that enables polymorphic rendering,
hit-testing, and editing.

### Phase 5.1 — PlotItem Abstraction (Refactor)

| Component | Description |
|-----------|-------------|
| PlotItem.h | Abstract base: Type enum, virtual paint/dataBounds/isVisible/name/primaryColor |
| LineSeries refactor | Inherits PlotItem, paint() moved from PlotRenderer |
| PlotScene refactor | vector<unique_ptr<PlotItem>>, backward-compat series() view |
| PlotRenderer | Polymorphic iteration via item->paint() |
| HitTester | Iterates items via dynamic_cast |
| WorkspaceFile | "type":"line" field, Phase 4 backward compat |

### Phase 5.2 — New Types

| Component | Description |
|-----------|-------------|
| ScatterSeries | 6 marker shapes (Circle/Square/Triangle/Diamond/Plus/Cross), filled/unfilled, 3-20px |
| BarSeries | Relative width * median X spacing, 2px minimum, y=0 baseline, optional outline |
| ChangeScatterPropertiesCommand | Bundled: color, shape, size, filled, name, visible |
| ChangeBarPropertiesCommand | Bundled: fillColor, outlineColor, barWidth, name, visible |
| ScatterPropertyDialog | Color, shape combo, size spin, filled check, name, visible |
| BarPropertyDialog | Fill color, outline color+none, bar width, name, visible |
| PlotCanvasDock type combo | Line/Scatter/Bar selector in toolbar |
| HitTester dispatch | item->type() routes to correct dialog |
| Legend swatches | Line segment / filled circle / filled rect per type |

---

## Human verification

### M5.1 gate (refactor) — passed 2026-04-11
Human verified all Phase 2-4 behaviors identical after refactor.
All 247 existing tests passed unchanged.

Human response: "Yes everything is good."

### M5.2 gate (new types) — passed 2026-04-11
Human verified scatter and bar creation, editing, and rendering.

Human response: "Yes perfectly show."

---

## Bug found and fixed

**autoRange only considered LineSeries**: PlotScene::autoRange()
built a temporary vector of LineSeries via dynamic_cast, ignoring
ScatterSeries and BarSeries entirely. Scatter markers were invisible
because the axis range didn't include their data bounds.

Fix: added Axis::extendAutoRange(lo, hi) to extend autoMin_/autoMax_
after the initial LineSeries autoRange pass. PlotScene::autoRange()
now calls extendAutoRange for each non-LineSeries item.

---

## Test results

- 275/275 tests pass (247 Phase 4 + 20 scatter/bar series + 8 commands)
- ASan + UBSan clean

---

## ADRs delivered

| ADR | Decision |
|-----|----------|
| ADR-028 | PlotItem abstract base with virtual paint/hit-test |
| ADR-029 | Series type immutable after creation |
| ADR-030 | Marker rendering via QPainter primitives, pixel sizing |
| ADR-031 | Bar layout: relative width, median spacing, 2px minimum |

---

## Lessons learned

### 1. Polymorphic autoRange must include all item types
The refactor preserved LineSeries-only autoRange from the original
code, but Phase 5.2's new types were invisible because autoRange
skipped them. Future PlotItem subclasses must be autoRange-included
by default.

### 2. Phase 5.1 refactor gate caught the issue
The M5.1 gate (247 tests unchanged, human verification) confirmed
the refactor was clean. The autoRange bug only appeared when
ScatterSeries was actually added in Phase 5.2, not during the
refactor itself.

### 3. Review in same commit enforced
This review is committed in the same commit as the closing STATUS
entry, as required by the Phase 3b/4 lesson.

---

## Exit checklist

- [x] Build clean (0 warnings)
- [x] 275 tests pass under ASan+UBSan
- [x] M5.1: all 247 existing tests passed unchanged
- [x] M5.1: human verified Phase 2-4 identical
- [x] M5.2: scatter and bar types work
- [x] M5.2: double-click opens correct dialog per type
- [x] M5.2: legend shows type-specific swatches
- [x] M5.2: column picker type combo works
- [x] ADR-028-031 committed
- [x] This review in SAME commit as STATUS close
- [x] vphase-5 tag (this commit)
