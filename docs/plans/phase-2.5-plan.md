# Phase 2.5 Plan — Consolidation and Phase 3 Foundation

> Reference: `docs/specs/phase-2.5-spec.md`

## Task Dependency Graph

```
  ┌──────────────────┐  ┌─────────────────┐  ┌──────────────────┐
  │T1 PlotRegistry + │  │T2 plot/ unit    │  │T3 ADRs 013-017  │
  │ EventBus events  │  │ tests (QA)      │  │ (Architect)      │
  │  (Backend)       │  │                 │  │                  │
  └────────┬─────────┘  └────────┬────────┘  └────────┬─────────┘
           │                     │                     │
           └─────────────┬───────┘─────────────────────┘
                         ▼
              ┌─────────────────────┐
              │T4 Phase 2 review    │
              │T5 architecture.md   │
              │ (Architect)         │
              └──────────┬──────────┘
                         ▼
              ┌─────────────────────┐
              │T6 README + STATUS   │
              │ (Docs)              │
              └──────────┬──────────┘
                         ▼
              ┌─────────────────────┐
              │Human re-verification│
              │ (7 interaction      │
              │  checks)            │
              └─────────────────────┘
```

## Critical path

T1 (PlotRegistry integration) → merge → human re-verification.
Other tasks can finish in any order and don't block verification.

---

## Tasks

### T1 — PlotRegistry + EventBus Events (Backend, Size: M, ~2h)

**Owner**: backend

**Files to create**:
- `src/lumen/core/PlotRegistry.h`
- `src/lumen/core/PlotRegistry.cpp`
- `tests/unit/test_plot_registry.cpp`

**Files to modify**:
- `src/lumen/core/EventBus.h` — add 4 enum entries
- `src/lumen/core/CMakeLists.txt` — add PlotRegistry sources
- `src/lumen/ui/PlotCanvasDock.h` / `.cpp` — accept PlotRegistry*,
  call registerPlot on document bind
- `src/lumen/ui/MainWindow.h` / `.cpp` — construct PlotRegistry,
  pass to PlotCanvasDock
- `src/lumen/app/Application.h` / `.cpp` — own PlotRegistry
  (if owned at Application level; alternatively MainWindow)
- `tests/unit/CMakeLists.txt` — add test_plot_registry.cpp

**Subtasks**:
- T1.1: Add 4 Event enum entries to EventBus.h (PlotCreated,
  PlotViewportChanged, PlotCrosshairMoved, PlotColumnsChanged)
  with payload convention comments.
- T1.2: Implement PlotRegistry — QObject, register/unregister/
  plotFor/clear/count. Connects QWidget::destroyed for auto-cleanup.
  Emits signals + publishes EventBus::PlotCreated.
- T1.3: Wire into PlotCanvasDock — takes PlotRegistry* in
  constructor (or setter). Calls registerPlot(documentPath,
  canvas_) when setDataFrame is called.
- T1.4: Wire into MainWindow/Application — construct PlotRegistry,
  pass to PlotCanvasDock.
- T1.5: Unit tests (≥5): register, lookup, unregister, destroyed
  auto-cleanup, count.
- T1.6: Verify auto-plot flow still works (build + run with
  real CSV).

**Acceptance criteria**:
- 5+ unit tests pass under ASan+UBSan
- EventBus.h has 4 new entries
- PlotCanvasDock registers its canvas on document bind
- Existing auto-plot flow still works (no regression)

**Dependencies**: none (can start immediately)

---

### T2 — Plot Module Unit Tests (QA, Size: M, ~2h)

**Owner**: qa

**Files to modify** (tests already exist from Phase 2 Round 1):
- `tests/unit/test_nice_numbers.cpp` — add ≥5 new tests
- `tests/unit/test_coordinate_mapper.cpp` — add ≥4 new tests
- `tests/unit/test_view_transform.cpp` — add ≥3 new tests
- `tests/unit/test_line_series.cpp` — add ≥3 new tests
- `tests/unit/test_axis.cpp` — add ≥3 new tests

**New test cases to add**:

NiceNumbers:
- [0, 1] range — ticks at 0.0, 0.2, 0.4, 0.6, 0.8, 1.0
- [0, 1e-6] very small range — produces positive tick count
- [0, 1e9] very large range — produces reasonable tick count
- [-50, 50] negative crossing zero — zero is a tick value
- Degenerate: min == max → produces at least 1 tick

CoordinateMapper:
- dataToPixel/pixelToData roundtrip within 1e-10
- Y-axis inversion: larger Y data → smaller Y pixel
- Degenerate: zero-width data range → no crash
- Pixel rect at non-origin position

ViewTransform:
- Pan shifts range by exact amount
- Zoom at center: center point unchanged after zoom
- Axis-specific zoom: zoomX doesn't change Y range

LineSeries:
- dataRange with all NaN → degenerate range (no crash)
- Single point → one polyline with one point
- Multiple NaN gaps → correct segment count

Axis:
- autoRange from series adds 5% padding
- Manual setRange overrides autoRange
- ticks() from series union covers both series

**Acceptance criteria**:
- ≥18 new test cases added
- Total test count ≥90
- All pass under ASan+UBSan

**Dependencies**: none (can start immediately, parallel with T1)

---

### T3 — ADRs 013–017 (Architect, Size: M, ~1h)

**Owner**: architect

**Files to create**:
- `docs/decisions/ADR-013-plot-margins-hardcoded.md`
- `docs/decisions/ADR-014-coordinate-mapper-precision.md`
- `docs/decisions/ADR-015-nice-numbers-algorithm.md`
- `docs/decisions/ADR-016-interaction-inline-in-plotcanvas.md`
- `docs/decisions/ADR-017-crosshair-cursor-not-nearest.md`

**Acceptance criteria**:
- Each ADR has Status, Context, Decision, Consequences, Alternatives
- ADR-013 explicitly names Phase 4 as the refactor target
- ADR-017 explicitly names Phase 4 as the upgrade trigger

**Dependencies**: none (can start immediately, parallel with T1, T2)

---

### T4 — Phase 2 Review (Architect, Size: S, ~30min)

**Owner**: architect

**File to create**:
- `docs/reviews/phase-2-review.md`

**Content**:
- What shipped (7 deliverables from phase-2-spec.md)
- What works (human's 7 interaction checks, verbatim)
- What was deferred (hardcoded margins, nearest-point crosshair,
  HitTester, InteractionController extraction)
- Lessons learned (agent permission issues, direct implementation
  by coordinator, fontconfig ASan false positives)
- Exit checklist

**Acceptance criteria**:
- Honest accounting of what was delivered vs. spec
- Human verification evidence included

**Dependencies**: T1, T2, T3 (review should reflect final state)

---

### T5 — Architecture Doc Phase 2 Section (Architect, Size: S, ~30min)

**Owner**: architect

**File to modify**:
- `docs/architecture.md`

**Content to add**:
- Phase 2 section: plot pipeline as implemented
  (PlotScene → PlotRenderer → QPainter, PlotCanvas mouse events)
- PlotRegistry integration
- Known tech debt (hardcoded margins, inline interaction)

**Dependencies**: T1 (PlotRegistry must be defined before
documenting it)

---

### T6 — README + STATUS.md (Docs, Size: S, ~30min)

**Owner**: docs

**Files to modify**:
- `README.md` — add plot capability description, update status
  to "Phase 2 — Plot Engine"
- `.lumen-ops/STATUS.md` — Phase 2.5 closing entry

**Acceptance criteria**:
- README reflects that Lumen can plot data interactively
- STATUS.md has closing entry with test count and verification

**Dependencies**: T1–T5 (after all other tasks merged)

---

## Parallel Execution Schedule

```
Round 1 (parallel, ~2h):
  Backend:   T1 (PlotRegistry + EventBus events)
  QA:        T2 (plot/ unit tests)
  Architect: T3 (ADRs 013-017)

Round 2 (~30min, sequential, Architect):
  T4 (Phase 2 review)
  T5 (architecture.md update)

Round 3 (~30min):
  Docs: T6 (README + STATUS)

Round 4:
  Human re-verification (7 interaction checks)
```

**Total wall time**: 3–4 hours.

## Risks

- **T1 PlotRegistry integration might break existing auto-plot
  flow**. Backend agent must run the app with real CSV and verify
  auto-plot still works before committing. If regression found,
  fix before merge.
- **T2 test additions might expose pre-existing bugs in
  CoordinateMapper or NiceNumbers**. Fix in Phase 2.5 if trivial
  (< 30 min). Defer with a documented issue if complex.

## Phase 1 Lessons Applied

- QA exclusively owns test files. Backend does not create
  overlapping fixtures or tests.
- Coordinator commits on behalf of agents blocked by sandbox
  permissions.
- Build verification (QT_QPA_PLATFORM=offscreen) required before
  every commit.
