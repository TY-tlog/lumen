# Phase 6 Plan — Universal Data Foundation

> Reference: `docs/specs/phase-6-spec.md`

## Tabular Representation Decision

**Option B: TabularBundle** — a convenience class grouping multiple
rank-1 Datasets sharing a row dimension. Each column is a true
rank-1 Dataset with its own name, unit, and coordinate array.
TabularBundle is NOT a Dataset subclass; it holds
`vector<shared_ptr<Dataset>>` and provides column-name lookup.

Rationale: cleaner separation (each column has independent type,
unit, and memory policy), natural fit for plotting (LineSeries
takes two rank-1 Datasets), and independent per-column chunking
for memory management. DocumentRegistry stores TabularBundle as a
top-level data object.

All migration code in this plan follows Option B consistently.

## Hard rules

1. **Review in same commit**: docs/reviews/phase-6-review.md MUST
   be WRITTEN AND COMMITTED IN THE SAME COMMIT as the closing
   .lumen-ops/STATUS.md entry. This rule appears verbatim in T27.

2. **M6.2 is non-negotiable**: all 275 existing tests pass
   UNCHANGED after DataFrame deletion. Byte-identical rendered
   output. M6.2 BLOCKS T13.

3. **v1 workspace backward compat**: Phase 4/5 .lumen.json files
   load correctly. Dedicated test in T12.

4. **Unit grammar**: "m" = meter, "mV" = millivolt, "mm" =
   millimeter. SI prefix form. Documented in ADR-034.

---

## Sub-phase Structure

```
6.1 Dataset Core    → M6.1 gate
6.2 DataFrame Migration → M6.2 gate (HARD BLOCK)
6.3 I/O + Memory    → M6.3 gate
6.4 UI + Closing    → M6.4 gate → T27 close
```

---

## Sub-phase 6.1 — Dataset Core (no I/O)

### T1 — Unit + Dimension (Backend, Size: L)

**Owner**: backend

**Files to create**:
- `src/lumen/data/Unit.h` / `.cpp`
- `src/lumen/data/Dimension.h`
- `src/lumen/data/CoordinateArray.h` / `.cpp`
- `tests/unit/test_unit_parsing.cpp`
- `tests/unit/test_unit_arithmetic.cpp`
- `tests/unit/test_unit_conversion.cpp`
- `tests/unit/test_dimension.cpp`
- `tests/unit/test_coordinate_array.cpp`

**Unit class**:
- 7 SI base dimensions stored as `std::array<int, 7>`
- `scaleToSI()` — conversion factor (e.g., mV → V = 0.001)
- `parse("mV")` → builds Unit from symbol string
- Grammar: SI prefixes (k, M, G, m, μ, n, p) + base symbols
  (V, A, s, m [meter], kg, mol, cd, K). "m" alone = meter.
  "mV" = milli + V. "mm" = milli + m. ADR-034 documents this.
- Operator overloads: `*`, `/`, `pow(int)`
- `isCompatible()`: same dimension exponents
- `convert(value, target)`: scale between compatible units
- Pre-registered: V, mV, A, nA, s, ms, m, mm, Hz, Ω, Pa, N, J, W

**Dimension struct**: name (QString), unit (Unit), length (size_t),
coordinates (CoordinateArray).

**CoordinateArray**: regular (start + step + count) or irregular
(explicit vector<double>). `valueAt(index)`, `indexOf(value)` for
label-based lookup.

**Tests**: parse "mV", "m/s^2", "kg*m^2/s^3"; arithmetic V=m*a;
conversion mV→V, s→ms; dimension equality/mismatch; coordinate
regular vs irregular, label lookup.

**Acceptance**: ≥15 tests pass under ASan+UBSan.

**Dependencies**: none (day 1)

---

### T2 — Dataset Abstract Base (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/data/Dataset.h` / `.cpp`
- `tests/unit/test_dataset_base.cpp`

**Dataset : QObject**:
```cpp
class Dataset : public QObject {
    Q_OBJECT
public:
    enum class StorageMode { InMemory, Chunked };
    virtual ~Dataset() = default;
    virtual QString name() const = 0;
    virtual std::vector<Dimension> dimensions() const = 0;
    virtual Unit valueUnit() const = 0;
    virtual StorageMode storageMode() const = 0;
    virtual std::size_t rank() const = 0;
    virtual std::vector<std::size_t> shape() const = 0;
    virtual std::size_t sizeBytes() const = 0;
    virtual double valueAt(const std::vector<std::size_t>& index) const = 0;
signals:
    void changed();
    void coordinatesChanged();
};
```

**Tests**: construct a mock concrete subclass, verify rank/shape/
signals.

**Acceptance**: Dataset compiles, signals fire, ≥3 tests.

**Dependencies**: T1 (Dimension, Unit)

---

### T3 — TabularBundle (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/data/TabularBundle.h` / `.cpp`
- `src/lumen/data/Rank1Dataset.h` / `.cpp` (concrete rank-1)
- `tests/unit/test_tabular_bundle.cpp`
- `tests/unit/test_rank1_dataset.cpp`

**Rank1Dataset : Dataset**: concrete rank-1 Dataset backed by a
`vector<double>` (or int64/QString). Name, unit, one Dimension
(the shared row dimension).

**TabularBundle**: NOT a Dataset subclass. Holds
`vector<shared_ptr<Rank1Dataset>>` all sharing the same row
Dimension. Provides:
- `addColumn(shared_ptr<Rank1Dataset>)`
- `column(int index)` / `columnByName(QString)`
- `columnCount()` / `rowCount()`
- `columnNames()` → QStringList

TabularBundle is the v2 replacement for DataFrame. The v1 Column
class can be adapted or its data extracted into Rank1Dataset.

**Tests**: multi-column bundle, shared row dim, column lookup by
name, row count consistency.

**Acceptance**: ≥6 tests.

**Dependencies**: T2 (Dataset base)

---

### T4 — Grid2D + Volume3D (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/data/Grid2D.h` / `.cpp`
- `src/lumen/data/Volume3D.h` / `.cpp`
- `tests/unit/test_grid2d.cpp`
- `tests/unit/test_volume3d.cpp`

**Grid2D : Dataset**: rank 2, two Dimensions (x, y), stores 2D
array of doubles. valueAt({i, j}). sizeBytes = rows * cols * 8.

**Volume3D : Dataset**: rank 3, three Dimensions (x, y, z), stores
3D array. Memory-manager-aware (volumes are often >100 MB).

**Tests**: construction, valueAt, sizeBytes, rank/shape.

**Acceptance**: ≥6 tests.

**Dependencies**: T2 (Dataset base)

---

### T5 — QA 6.1 Tests (QA, Size: S)

**Owner**: qa

Verify: all new tests from T1-T4 pass. All 275 existing tests
still pass (DataFrame unchanged at this point).

**Acceptance**: 275 + ≥30 new = ≥305 total.

**Dependencies**: T1-T4

---

### M6.1 — Gate

All 6.1 tests pass. 275 existing tests pass (DataFrame still
exists). Human confirms build is clean.

---

## Sub-phase 6.2 — DataFrame Migration (CRITICAL)

### T6 — DocumentRegistry Generalization (Backend, Size: M)

**Owner**: backend

**Files to modify**:
- `src/lumen/core/DocumentRegistry.h` / `.cpp`

Change from `shared_ptr<DataFrame>` to a variant or union type
that holds either `shared_ptr<TabularBundle>` or
`shared_ptr<Dataset>`:

```cpp
using DataObject = std::variant<
    std::shared_ptr<TabularBundle>,
    std::shared_ptr<Dataset>>;
```

Or simpler: store `shared_ptr<TabularBundle>` for now (Phase 6.2
only handles tabular data; Grid2D/Volume3D registration added in
6.3).

All callers of `document(path) → DataFrame*` updated to
`documentBundle(path) → TabularBundle*`.

**Acceptance**: all callers compile, tests pass.

**Dependencies**: M6.1 gate

---

### T7 — PlotItem Constructor Migration (Backend, Size: L)

**Owner**: backend

**Files to modify**:
- `src/lumen/plot/LineSeries.h` / `.cpp`
- `src/lumen/plot/ScatterSeries.h` / `.cpp`
- `src/lumen/plot/BarSeries.h` / `.cpp`
- `src/lumen/ui/PlotCanvasDock.cpp`

Change constructors from `(const Column* xCol, const Column* yCol,
...)` to `(shared_ptr<Rank1Dataset> xDs, shared_ptr<Rank1Dataset>
yDs, ...)`.

Internally, each series reads data from `xDs->doubleData()` (or
equivalent accessor on Rank1Dataset). The rendering logic stays
the same — it just reads from a different data source.

PlotCanvasDock::rebuildPlot() creates series from
TabularBundle columns instead of DataFrame columns.

**CRITICAL**: rendered output must be pixel-identical. The data
values are the same; only the container changed.

**Acceptance**: all plot rendering identical, tests pass.

**Dependencies**: T6

---

### T8 — WorkspaceFile Migration (Backend, Size: M)

**Owner**: backend

**Files to modify**:
- `src/lumen/core/io/WorkspaceFile.h` / `.cpp`

captureFromScene: reads from Rank1Dataset instead of Column.
Serialization format unchanged (still JSON v1).

applyToScene: reads column names from JSON, resolves against
TabularBundle columns instead of DataFrame columns.

**v1 backward compat**: if JSON has "xColumn": 0 (integer index),
resolve by index from TabularBundle. If has "xColumn": "time_ms"
(string name), resolve by name. Both paths must work.

**Acceptance**: v1 workspace loads correctly, new workspace saves
correctly.

**Dependencies**: T6, T7

---

### T9 — Delete DataFrame (Backend, Size: S)

**Owner**: backend

**Files to DELETE**:
- `src/lumen/data/DataFrame.h`
- `src/lumen/data/DataFrame.cpp`
- `src/lumen/data/Column.h`
- `src/lumen/data/Column.cpp`
- `src/lumen/data/ColumnType.h`

**Files to modify**:
- `src/lumen/data/CMakeLists.txt` — remove deleted files
- Any remaining references

**CRITICAL**: This is the point of no return. Only do this after
T6, T7, T8 are complete and all callers are migrated.

**Acceptance**: build succeeds with no DataFrame references.

**Dependencies**: T6, T7, T8

---

### T10 — CsvReader → TabularBundle (Backend, Size: M)

**Owner**: backend

**Files to modify**:
- `src/lumen/data/CsvReader.h` / `.cpp`

CsvReader currently returns DataFrame. Change to return
TabularBundle (or provide a new method `readFileAsBundle`).

Each column becomes a Rank1Dataset with:
- Name from CSV header
- Unit parsed from header if format "name (unit)" detected, else
  dimensionless
- Row coordinate: shared row Dimension with regular coordinates
  (0, 1, 2, ...)

**Acceptance**: CSV loading produces TabularBundle, all downstream
code works.

**Dependencies**: T3 (TabularBundle), T9 (DataFrame deleted)

---

### T11 — Byte-Identical Render Regression (QA, Size: M)

**Owner**: qa

**Files to create**:
- `tests/integration/test_render_regression.cpp`

**Method**:
1. Before T6 starts: render the reference electrophysiology CSV
   to a QImage using current code. Save as reference PNG.
2. After T9 completes: render the same CSV to a QImage using
   migrated code. Compare pixel-by-pixel.
3. Assert: zero pixel differences.

(In practice: the reference image is committed as a test fixture.
The test renders and compares.)

**Acceptance**: pixel-equal render. If any difference, fix the
migration, NOT the test.

**Dependencies**: T9 (migration complete)

---

### T12 — v1 Workspace Load Test (QA, Size: S)

**Owner**: qa

**Files to create**:
- `tests/integration/test_v1_workspace_load.cpp`
- `tests/fixtures/tiny/v1_workspace.lumen.json` (fixture)

Create a v1 workspace fixture (Phase 4/5 format with column-index
fields, no Dataset references). Load it after migration. Verify all
edits restored: line color, axis labels, title, series properties.

**Acceptance**: v1 workspace loads correctly in migrated code.

**Dependencies**: T8 (WorkspaceFile migration)

---

### M6.2 — Gate (HARD BLOCK — NO T13 UNTIL PASSED)

**ALL of the following must be true**:
1. Build clean, 0 warnings
2. All 275 existing tests pass UNCHANGED
3. T11 render regression: pixel-equal
4. T12 v1 workspace loads correctly
5. Human verifies: open real CSV, plot identical, save workspace,
   reopen, export PNG — all identical to Phase 5

**If ANY fails**: fix before proceeding. T13 CANNOT start.

---

## Sub-phase 6.3 — I/O and Memory

### T13 — DatasetLoader Interface + CsvLoader (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/data/io/DatasetLoader.h` / `.cpp`
- `src/lumen/data/io/CsvLoader.h` / `.cpp`
- `src/lumen/data/io/LoaderRegistry.h` / `.cpp`
- `src/lumen/data/io/CMakeLists.txt`

**DatasetLoader interface**:
```cpp
class DatasetLoader {
public:
    virtual ~DatasetLoader() = default;
    virtual QStringList supportedExtensions() const = 0;
    virtual std::unique_ptr<TabularBundle> loadTabular(
        const QString& path) = 0;
    virtual std::unique_ptr<Dataset> loadDataset(
        const QString& path) = 0;
    virtual bool canLoad(const QString& path) const;
};
```

**LoaderRegistry**: singleton, maps extensions to loaders.
Plugin-ready (Phase 16).

**CsvLoader**: wraps CsvReader, produces TabularBundle.

**Acceptance**: CSV loads through new loader path. ≥3 tests.

**Dependencies**: M6.2 gate

---

### T14 — Hdf5Loader + NetCDFLoader (Backend, Size: L)

**Owner**: backend

**Dependencies**: apt packages `libhdf5-dev`, `libnetcdf-dev`

**Hdf5Loader**: reads HDF5 datasets into Grid2D/Volume3D/
TabularBundle depending on rank. Uses HDF5 C++ API.

**NetCDFLoader**: reads NetCDF files similarly. Uses netcdf-c++.

**Acceptance**: ≥4 tests each (1D → TabularBundle, 2D → Grid2D,
attribute reading, large file → chunked mode).

**Dependencies**: T13 (loader interface)

---

### T15 — ParquetLoader + ZarrLoader (Backend, Size: L)

**Owner**: backend

**Dependencies**: `libarrow-dev`, custom Zarr reader

**Acceptance**: ≥3 tests each.

**Dependencies**: T13

---

### T16 — TiffStackLoader + JsonLoader (Backend, Size: M)

**Owner**: backend

**Dependencies**: `libtiff-dev`

**TiffStackLoader**: TIFF image stack → Volume3D.
**JsonLoader**: structured JSON → TabularBundle.

**Acceptance**: ≥2 tests each.

**Dependencies**: T13

---

### T17 — MatLoader + NumpyLoader (Backend, Size: M)

**Owner**: backend

**Dependencies**: `libmatio-dev`

**MatLoader**: .mat v5/v7.3 via matio library.
**NumpyLoader**: .npy binary format, custom parser.

**Acceptance**: ≥2 tests each.

**Dependencies**: T13

---

### T18 — MemoryManager (Backend, Size: M)

**Owner**: backend

**Files to create**:
- `src/lumen/data/MemoryManager.h` / `.cpp`
- `tests/unit/test_memory_manager.cpp`

**MemoryManager**: singleton. Decides InMemory vs Chunked based on
100 MB threshold. LRU cache for chunks. Configurable budget
(default 4 GB). currentUsageBytes(), setBudget().

**Acceptance**: ≥5 tests (budget enforcement, LRU eviction,
threshold decision).

**Dependencies**: T2 (Dataset StorageMode)

---

### T19 — QA I/O Tests (QA, Size: M)

**Owner**: qa

Round-trip tests per loader. Large-file memory test.

**Acceptance**: ≥20 new tests, total ≥350.

**Dependencies**: T14-T18

---

### M6.3 — Gate

All 9 loaders functional. Memory budget enforced. Human loads an
HDF5 file.

---

## Sub-phase 6.4 — UI Surface

### T20 — DataTableDock Grid2D/Volume3D Placeholders (Frontend, Size: S)

**Owner**: frontend

DataTableDock: if data is Grid2D, show "Grid2D: W × H (type),
ready for Phase 7 heatmap". Volume3D: similar placeholder.

**Dependencies**: M6.3

---

### T21 — File Open for All Formats (Frontend, Size: S)

**Owner**: frontend

MainWindow File → Open: extend filter to include all 9 formats.
Use LoaderRegistry to pick the right loader by extension.

**Dependencies**: T13 (LoaderRegistry)

---

### T22 — Sample Menu (Frontend, Size: M)

**Owner**: frontend

File → Open Sample submenu:
- "Sine 1D" → TabularBundle with sin(x) data
- "Gaussian 2D" → Grid2D with gaussian peak
- "Mandelbrot" → Grid2D with Mandelbrot set
- "Volume Sphere" → Volume3D with sphere function

Each creates synthetic data in-memory, registers in
DocumentRegistry.

**Dependencies**: T3, T4 (data types)

---

### T23 — Memory Status Bar (Frontend, Size: S)

**Owner**: frontend

Status bar: "342 MB / 4 GB" updated on timer or on data change.
Reads from MemoryManager.

**Dependencies**: T18

---

### T24 — Settings Memory Budget (Frontend, Size: S)

**Owner**: frontend

Settings dialog or preferences: slider for memory budget (256 MB
to system RAM). Calls MemoryManager::setBudget().

**Dependencies**: T18

---

### T25 — QA UI Tests (QA, Size: S)

Sample menu tests, HDF5 integration test.

**Dependencies**: T20-T24

---

### T26 — Human Real Data (Human, Size: S)

Human opens a real HDF5 or NetCDF scientific file. Verifies load,
memory stays within budget.

---

### M6.4 — Gate

Human verification passes.

---

## Closing

### T27 — Docs Closing (REVIEW AND STATUS IN SAME COMMIT)

**Owner**: docs

**HARD RULE (verbatim, Phase 3b/4/5 lesson)**: docs/reviews/
phase-6-review.md MUST be WRITTEN AND COMMITTED IN THE SAME COMMIT
as the closing .lumen-ops/STATUS.md entry. This is one commit, one
`git add`, one `git commit`. Not a separate task. Not a follow-up.
The coordinator MUST enforce this. If the review is missing from
the commit, the commit MUST be amended before push.

**Files in ONE commit**:
- `docs/reviews/phase-6-review.md`
- `.lumen-ops/STATUS.md` (closing entry)
- `README.md` (universal data foundation, 9 formats)
- `src/lumen/data/CLAUDE.md` (rewrite: Dataset, TabularBundle,
  Grid2D, Volume3D, Unit, Dimension, CoordinateArray)
- `src/lumen/data/io/CLAUDE.md` (new: 9 loaders, LoaderRegistry,
  MemoryManager)

After commit: `git tag vphase-6 && git push origin main --tags`.

**Dependencies**: M6.4 gate

---

## Parallel Execution Schedule

```
Sub-phase 6.1 (parallel, ~6h):
  Backend: T1 (Unit+Dimension) + T2 (Dataset base) + T3 (TabularBundle) + T4 (Grid2D+Volume3D)
  QA: T5 (ongoing as T1-T4 land)

Gate M6.1

Sub-phase 6.2 (sequential, ~8h):
  Backend: T6 (DocumentRegistry) → T7 (PlotItem ctors) → T8 (WorkspaceFile) → T10 (CsvReader) → T9 (DELETE DataFrame)
  QA: T11 (render regression) + T12 (v1 workspace)

Gate M6.2 (HARD BLOCK)

Sub-phase 6.3 (parallel, ~10h):
  Backend: T13 (loader interface) → T14-T17 (parallel per format) + T18 (MemoryManager)
  QA: T19 (ongoing)

Gate M6.3

Sub-phase 6.4 (parallel, ~4h):
  Frontend: T20-T24
  QA: T25
  Human: T26

Gate M6.4

Closing (~30min):
  T27 (review+STATUS SAME commit, then tag)
```

**Total wall time**: ~30-36 hours across sub-phases.

## Risks

- **DataFrame migration breadth**: T6-T10 touch nearly every file
  in the project. Careful mechanical replacement needed.
- **HDF5/NetCDF library linking**: may need apt install at build
  time. Test early in T14.
- **Unit parser complexity**: "m" = meter, "ms" = millisecond,
  "m/s" = meters per second. Grammar must be unambiguous.
- **Memory manager LRU under pressure**: test with 1.5× budget.
- **v1 workspace column-index vs column-name resolution**: both
  paths must work in T8.

## Lessons Applied

- Review in same commit as STATUS (Phase 3b/4/5).
- M6.2 hard gate with byte-identical render test (Phase 5.1
  pattern applied to data migration).
- v1 workspace backward compat tested with real fixture.
- Sub-phase gates prevent cascading failures.
