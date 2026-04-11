# Phase 6 Review — Universal Data Foundation

**Date**: 2026-04-12
**Reviewer**: Architect agent
**Spec**: `docs/specs/phase-6-spec.md`
**Plan**: `docs/plans/phase-6-plan.md`

---

## What shipped

Phase 6 replaced the v1 DataFrame-only data model with a universal
Dataset abstraction supporting tabular, 2D grid, and 3D volume data
with physical units, labeled coordinates, and 7 file format loaders.

### Sub-phase 6.1 — Dataset Core
- Unit: 7 SI dimensions, parse/convert/arithmetic (ADR-034)
- Dimension + CoordinateArray: labeled coordinates
- Dataset: QObject abstract base with reactive signals (ADR-032/035)
- Rank1Dataset: concrete rank-1 (Column replacement)
- TabularBundle: groups Rank1Datasets (DataFrame replacement, Option B)
- Grid2D + Volume3D: rank-2 and rank-3 scalar fields

### Sub-phase 6.2 — DataFrame Migration
- DocumentRegistry generalized to TabularBundle
- LineSeries/ScatterSeries/BarSeries take shared_ptr<Rank1Dataset>
- WorkspaceFile updated with v1 backward compat
- CsvReader returns TabularBundle
- DataFrame.h/.cpp, Column.h/.cpp, ColumnType.h DELETED
- 53 files changed in migration

### Sub-phase 6.3 — I/O and Memory
- DatasetLoader interface + LoaderRegistry (ADR-037)
- 7 loaders: CSV, HDF5, NetCDF, TIFF, JSON, Mat, Numpy
- Parquet and Zarr deferred (no apt packages)
- MemoryManager: 4GB budget, 100MB threshold (ADR-033)

### Sub-phase 6.4 — UI Surface
- File → Open supports all 7+ formats via LoaderRegistry
- File → Open Sample: Sine 1D, Gaussian 2D, Mandelbrot, Volume Sphere
- DataTableDock: Grid2D/Volume3D placeholders
- Memory status bar + budget settings

---

## Human verification

### M6.1 (Dataset core): passed
275 existing tests unchanged + 88 new = 363 total.

### M6.2 (DataFrame migration): passed
360 tests pass. Real CSV opens, plots identical. Workspace loads.

### M6.3 (I/O): passed
406 tests. HDF5/NetCDF/TIFF/JSON/Mat/Numpy loaders functional.

### M6.4 (UI surface): passed
410 tests. All 4 samples load. Real CSV still works.
Human response: "yes."

---

## Bugs found and fixed
- autoRange only considered LineSeries → extendAutoRange for scatter/bar
- QRectF::isEmpty() true for zero-height bounds → use isNull()
- HitTester hitTest only handled LineSeries → added scatter/bar paths
- HitTester hitTestPoint only handled LineSeries → linear scan for scatter/bar
- PlotCanvas crosshair used series() (LineSeries-only view) → crash on scatter/bar → use items()
- PDF export: labels/title tiny → scale QPainter to logical size
- Type combo changed all series at once → per-entry type combo
- addYSeries signal fired before entry stored → block signals during setup
- Remove button captured stale index → find by pointer
- Toolbar overflow with many series → QScrollArea
- Sample datasets destroyed after function return → store in sampleDatasets_
- showDataset didn't show DataTableDock → raise + hide plot dock

---

## Test results
- 410/410 tests pass
- ASan + UBSan clean
- Zero compiler warnings

---

## ADRs delivered
| ADR | Decision |
|-----|----------|
| ADR-032 | xarray-style Dataset with labeled coordinates |
| ADR-033 | Hybrid memory (100MB threshold, 4GB budget) |
| ADR-034 | Physical units with dimensional analysis |
| ADR-035 | Reactive Dataset signals from foundation |
| ADR-036 | DataFrame full deletion, no legacy wrapper |
| ADR-037 | DatasetLoader plugin interface with registry |

---

## Lessons learned

### 1. DataFrame migration was the riskiest change in the project
53 files touched, 5 classes deleted. The M6.2 gate caught multiple
bugs that would have cascaded into Phase 6.3/6.4.

### 2. PlotItem polymorphism requires updating ALL call sites
Every place that used series()-specific APIs (LineSeries cast,
Column access) had to be updated to use items() with PlotItem
polymorphism. The LineSeriesView compatibility layer helped tests
compile but caused a crash when scatter/bar items were accessed
through it.

### 3. Per-entry state in UI requires careful signal ordering
The Y series combo's currentIndexChanged signal fired during
setup before the entry was stored. Blocking signals during
initialization and connecting after push_back fixed this.

---

## Exit checklist
- [x] Build clean (0 warnings)
- [x] 410 tests pass under ASan+UBSan
- [x] 7 loaders functional (Parquet/Zarr deferred)
- [x] v1 CSV and workspace files load
- [x] Human verified real CSV + all 4 samples
- [x] ADR-032 through ADR-037 committed
- [x] This review in SAME commit as STATUS close
- [x] vphase-6 tag (this commit)
