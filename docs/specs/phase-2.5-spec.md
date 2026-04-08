# Phase 2.5 — Consolidation and Phase 3 Foundation

## Context

Phase 2 MVP is functionally complete. The human verified all 7
interaction checks (open CSV → auto line plot → pan → wheel zoom →
box zoom → double-click reset → crosshair readout) with their real
electrophysiology CSV (3499 rows × 9 columns, contains NaN).

However, several gaps remain:

- **Test count is still at Phase 1 level (72 unit + 5 integration)**.
  No Phase 2 module unit tests were added beyond 3 PlotRenderer
  image tests. The plot/ module has ~1,100 lines of C++ with
  minimal test coverage.
- **No Phase 2 ADRs**. Decisions about coordinate precision,
  tick algorithm, inline interaction, crosshair behavior, and
  hardcoded margins were made implicitly during implementation.
- **No Phase 2 review**. STATUS.md has no closing entry for Phase 2.
- **Core services required as Phase 3 foundation are missing**.
  PlotRegistry (tracking which PlotCanvas belongs to which
  document) and Phase 2 EventBus events (PlotCreated,
  PlotViewportChanged, PlotCrosshairMoved, PlotColumnsChanged)
  are needed for undo/redo and cross-module plot event propagation.

## Goal

Close Phase 2 cleanly. Add the minimal core services that Phase 3
will depend on. Document already-made decisions as ADRs. Do NOT
refactor working code.

## Active agents

- Architect (ADRs, review, architecture update)
- Backend (PlotRegistry, EventBus events)
- QA (plot/ unit tests)
- Docs (README, STATUS.md)

## Deliverables

### D1 — PlotRegistry (`src/lumen/core/`)

- `PlotRegistry.h` / `.cpp` — maps document path (QString) to
  PlotCanvas* (non-owning pointer).
- Methods: `registerPlot(QString path, QWidget* canvas)`,
  `unregisterPlot(QString path)`, `plotFor(QString path) → QWidget*`,
  `clear()`, `count() → int`.
- Connects `PlotCanvas::destroyed` signal for automatic cleanup
  when a canvas widget is deleted.
- Signals: `plotRegistered(QString path)`,
  `plotUnregistered(QString path)`.
- Also publishes `EventBus::PlotCreated` on register.
- Owned by Application (or MainWindow), passed to PlotCanvasDock
  which calls `registerPlot` when a document is bound.
- Unit tests (≥5): register, lookup, unregister, destroyed
  auto-cleanup, count.

### D2 — EventBus Phase 2 events

Add four entries to the `Event` enum in `src/lumen/core/EventBus.h`:

```cpp
PlotCreated,          // payload: QString documentPath
PlotViewportChanged,  // payload: QString documentPath (future: QRectF viewport)
PlotCrosshairMoved,   // payload: QString documentPath (future: QPointF dataPos)
PlotColumnsChanged,   // payload: QString documentPath
```

Payload conventions documented as inline comments. Phase 2.5 only
emits `PlotCreated` (from PlotRegistry). The other three are added
for Phase 3 use but left unpublished for now.

### D3 — Plot module unit tests (≥18 new tests, target ≥90 total)

Owned by QA. Test files:

- `tests/unit/test_nice_numbers.cpp` (≥5 tests) — range [0,1],
  [0,1e-6], [0,1e9], negative crossing zero, tiny range,
  degenerate (min==max).
- `tests/unit/test_coordinate_mapper.cpp` (≥4 tests) —
  dataToPixel/pixelToData roundtrip, Y-axis inversion, empty pixel
  rect, degenerate data range.
- `tests/unit/test_view_transform.cpp` (≥3 tests) — pan, zoom at
  center, axis-specific zoom, reset.
- `tests/unit/test_line_series.cpp` (≥3 tests) — dataRange with
  NaN, empty columns, single point.
- `tests/unit/test_axis.cpp` (≥3 tests) — 5% padding, manual
  range override, tick generation from series union.

Note: some of these test files already exist from Phase 2 Round 1
(Backend created them). QA should review existing tests, add the
missing cases specified above, and ensure the total is ≥18 new
tests beyond what already exists.

All tests link only plot/ and data/; no UI dependency. All pass
under ASan+UBSan.

### D4 — ADRs 013–017

- ADR-013: Hardcoded plot margins as known tech debt (Phase 4 refactor)
- ADR-014: CoordinateMapper precision guarantees
- ADR-015: NiceNumbers 1-2-5 algorithm choice
- ADR-016: Interaction logic inline in PlotCanvas (not extracted)
- ADR-017: Crosshair shows cursor position, not nearest data point

### D5 — Phase 2 review

`docs/reviews/phase-2-review.md` — what shipped, what works
(human's 7 interaction checks), what was deferred, lessons learned,
exit checklist.

### D6 — README.md + architecture.md + STATUS.md updates

- README.md reflects Phase 2 complete (plot capability).
- `docs/architecture.md` has accurate Phase 2 section describing
  the plot pipeline as actually implemented.
- `.lumen-ops/STATUS.md` has Phase 2.5 closing entry.

## Explicit non-goals

- No refactoring of PlotCanvas, PlotCanvasDock, PlotRenderer.
- No extraction of HitTester, InteractionController, Legend as
  separate classes (ADR-016 documents the deferral).
- No replacement of hardcoded margins in PlotScene (ADR-013
  documents the deferral).
- No upgrade of crosshair from "cursor position" to "nearest
  data point" (ADR-017 documents the deferral).
- No new test fixtures.

## Acceptance criteria

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
    -DLUMEN_ENABLE_ASAN=ON -DLUMEN_ENABLE_UBSAN=ON
cmake --build build
cd build && ctest --output-on-failure
```

- Build clean: 0 warnings, 0 errors (Ubuntu Debug + Release)
- ≥90 tests passing under ASan+UBSan
- All 5 ADRs committed (ADR-013 through ADR-017)
- `docs/reviews/phase-2-review.md` committed
- README.md reflects Phase 2 complete status
- `docs/architecture.md` has accurate Phase 2 section
- `.lumen-ops/STATUS.md` has Phase 2.5 closing entry
- Human has re-verified the 7 interaction checks still work
  after Phase 2.5 changes (PlotRegistry integration touches
  MainWindow and PlotCanvasDock)

## Exit checklist

- [ ] Build clean (0 warnings, 0 errors)
- [ ] ≥90 tests green under ASan+UBSan
- [ ] 5 ADRs committed (ADR-013..017)
- [ ] Phase 2 review committed
- [ ] README.md updated
- [ ] architecture.md updated
- [ ] Human re-verification pass (7 interaction checks)
