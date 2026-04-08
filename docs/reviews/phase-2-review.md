# Phase 2 Review — Plot Engine (Line Plot)

**Date**: 2026-04-09
**Reviewer**: Architect agent
**Spec**: `docs/specs/phase-2-spec.md`
**Plan**: `docs/plans/phase-2-plan.md`

---

## What shipped

Phase 2 delivered a self-built interactive 2D line plot engine on
QPainter, integrated into the Lumen window with automatic plotting
on CSV open.

### Components delivered (mapped to spec deliverables)

| Spec deliverable | Implemented as | Status |
|-----------------|----------------|--------|
| D1 Plot object model | PlotScene, Axis, LineSeries, PlotStyle | Done |
| D2 Coordinate system | CoordinateMapper, ViewTransform, NiceNumbers | Done |
| D3 Rendering engine | PlotRenderer | Done |
| D4 PlotCanvas widget | PlotCanvas, PlotCanvasDock | Done |
| D5 Interaction | Inline in PlotCanvas (pan, zoom, box-zoom, reset, crosshair) | Done (simplified, ADR-016/017) |
| D6 Plot creation flow | Auto-plot on CSV open, column picker combos, multi-series | Done |
| D7 Tests | 33 plot unit tests + 3 renderer tests | Partial (Phase 2.5 adds more) |

### Files added (Phase 2)

**src/lumen/plot/** (11 files):
- NiceNumbers.h/.cpp — 1-2-5 tick algorithm
- CoordinateMapper.h/.cpp — data-pixel bidirectional mapping
- ViewTransform.h/.cpp — pan/zoom state
- LineSeries.h/.cpp — column references, NaN gap polylines
- PlotStyle.h — per-series visual properties
- Axis.h/.cpp — range, ticks, labels
- PlotScene.h/.cpp — scene container, layout
- PlotRenderer.h/.cpp — QPainter rendering

**src/lumen/ui/** (4 files):
- PlotCanvas.h/.cpp — widget + interaction
- PlotCanvasDock.h/.cpp — dock + column picker

**tests/unit/** (8 files):
- test_nice_numbers.cpp, test_coordinate_mapper.cpp,
  test_view_transform.cpp, test_line_series.cpp,
  test_axis.cpp, test_plot_scene.cpp, test_plot_renderer.cpp

---

## Human verification

Human verified all 7 interaction checks with real electrophysiology
CSV: automatic line plot rendering, pan via left-drag, wheel zoom
(Shift=X, Ctrl=Y), right-drag box zoom, double-click reset,
crosshair tooltip readout. All passed. Verification performed
2026-04-09.

Reference data: `wt0_cap100nM_d20251029_s002_before_1x_raw.csv`
(3499 rows, 9 columns, voltage trace with action potential spikes,
NaN at derivative boundary rows).

Human response: "Yes I can see great."

---

## What works

1. **Auto-plot**: Opening a CSV automatically plots the first two
   numeric columns (time_ms vs voltage_mV).
2. **Pan**: Left-drag moves the view smoothly.
3. **Zoom**: Scroll wheel zooms centered on cursor. Shift for X-only,
   Ctrl for Y-only.
4. **Zoom box**: Right-drag draws a rectangle, releases to zoom.
5. **Reset**: Double-click returns to auto-range.
6. **Crosshair**: Mouse hover shows data coordinates (cursor position
   in data space, 4 significant digits).
7. **Column picker**: Combo boxes for X and Y columns, Add Series
   button for multi-series with palette colors.
8. **NaN handling**: Derivative columns (dvdt_3pt_bwd) correctly show
   gaps at boundary NaN values.
9. **Nice ticks**: Voltage range (-80 to +40 mV) and time range
   (0 to 700 ms) produce readable tick labels.
10. **Apple-mood styling**: Plot background, grid, axis colors all
    from DesignTokens.

---

## What was deferred

| Item | Reason | Target | ADR |
|------|--------|--------|-----|
| Dynamic plot margins | Premature; needs QFontMetrics | Phase 4 | ADR-013 |
| Nearest-point crosshair snap | Needs HitTester | Phase 4 | ADR-017 |
| InteractionController extraction | 150 lines, manageable | Phase 4 | ADR-016 |
| PlotRegistry (document-to-canvas mapping) | Phase 3 foundation | Phase 2.5 | — |
| EventBus plot events | Phase 3 foundation | Phase 2.5 | — |
| Comprehensive plot/ unit tests | Phase 2 focused on delivery | Phase 2.5 | — |
| Scatter/bar/histogram | Spec non-goal | Phase 3 | — |
| Logarithmic axes | Spec non-goal | Phase 3 | — |
| Export to PNG/SVG | Spec non-goal | Phase 4 | — |
| Data decimation for large datasets | Spec non-goal | Phase 3 | — |

---

## Lessons learned

### 1. Agent permission issues require coordinator fallback
Subagents were repeatedly blocked by Write and Bash sandbox
permissions in Phase 2. The coordinator (main Claude Code session)
had to implement T4 (Axis+PlotScene), T5 (PlotRenderer), and
T6+T7 (PlotCanvas+PlotCanvasDock+column picker) directly. This
is workable but loses the parallel execution benefit.

**Mitigation for Phase 2.5+**: Grant all permissions to subagents
upfront, or accept that the coordinator will handle implementation
directly when agents are blocked.

### 2. Fontconfig LeakSanitizer false positives
QGuiApplication initialization triggers memory "leaks" in
libfontconfig (system library, not our code). This causes
PlotRenderer tests to fail under ASan's leak detector.

**Mitigation**: Use `ASAN_OPTIONS=detect_leaks=0` for tests that
create QGuiApplication, or suppress fontconfig specifically via
an ASan suppression file.

### 3. Test coverage lagged behind implementation
Phase 2 delivered ~1,500 lines of plot/ code with only 36 tests
(33 unit + 3 renderer). Backend agents created unit tests for their
classes (NiceNumbers, CoordinateMapper, ViewTransform, LineSeries),
but the coordinator-implemented classes (PlotRenderer, PlotCanvas,
PlotCanvasDock) received minimal or no tests.

**Mitigation**: Phase 2.5 adds >=18 additional tests. Future phases
should enforce "tests in the same commit as implementation."

### 4. Direct implementation by coordinator is faster but less reviewable
When the coordinator implements code directly (instead of via
agents), there is no PR review step. The code works (human-verified)
but hasn't been independently reviewed.

**Mitigation**: Phase 2.5's QA agent reviews existing code while
adding tests, providing retroactive coverage.

---

## Exit checklist

- [x] D1-D6 delivered (plot object model through column picker)
- [x] D7 partially delivered (36 tests, Phase 2.5 adds more)
- [x] Human verified 7 interaction checks with real data
- [x] Build clean: 0 warnings, ASan+UBSan clean (except fontconfig)
- [x] ADRs 013-017 documenting Phase 2 decisions (Phase 2.5)
- [x] This review written
- [ ] PlotRegistry + EventBus events (Phase 2.5 T1)
- [ ] Test count >= 90 (Phase 2.5 T2)
- [ ] README.md updated (Phase 2.5 T6)
