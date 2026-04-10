# Phase 6 — Universal Data Foundation

## Goal

Replace the v1 DataFrame-only data model with a universal
Dataset abstraction that represents all scientific data: tabular,
n-dimensional arrays with labeled coordinates and physical units,
with memory-efficient loading for large files. This is the
foundation for every v2 phase that follows (reactive plots, 3D,
scalar fields, non-Cartesian projections).

This phase is intentionally foundational. Visible changes are
minimal; internal changes are total.

## Design decisions (approved)

1. **Abstraction**: n-dimensional array + metadata (xarray model)
2. **Memory**: hybrid — in-memory below 100 MB, chunked with LRU
   cache above
3. **Coordinates**: labeled coordinates + physical units with
   dimensional analysis
4. **I/O range**: CSV, HDF5, NetCDF, Parquet, Zarr, TIFF stack,
   JSON, .mat, .npy
5. **Reactive foundation**: Dataset emits changed() signal and
   has observable coordinates from day one
6. **Legacy**: DataFrame fully replaced by TabularDataset;
   v1 code migrated
7. **Visibility**: minimal UI surface (HDF5/NetCDF open works)
   + synthetic sample menu

## Active agents

Architect, Backend, Frontend, QA, Integration, Docs.

## Scope: four internal sub-phases

- **6.1** Dataset core + units + coordinates (no I/O yet)
- **6.2** DataFrame migration to TabularDataset
- **6.3** File I/O (9 formats) + memory manager
- **6.4** UI surface (Open HDF5, sample menu) + Phase 7 readiness

Human verification gate at the end of each sub-phase.

## Phase 6.1 — Dataset Core

### data/Dataset.{h,cpp}

Abstract base:
namespace lumen::data {
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

// Value access
virtual double valueAt(const std::vector<std::size_t>& index) const = 0;
virtual QVariant metadataAt(
    const std::vector<std::size_t>& index) const;

// Slice / subset by label or index
virtual std::unique_ptr<Dataset> sliceByLabel(
    const std::map<QString, std::pair<double, double>>& ranges) const = 0;
signals:
void changed();
void coordinatesChanged();
};
}

### data/Dimension.{h,cpp}
struct Dimension {
QString name;        // "time", "x", "voltage"
Unit unit;           // seconds, meters, volts
std::size_t length;
CoordinateArray coordinates;  // actual coordinate values
};

### data/Unit.{h,cpp}

Dimensional analysis engine:
class Unit {
public:
static Unit dimensionless();
static Unit parse(const QString& symbol);  // "mV", "m/s^2"
// 7 SI base dimensions: length, mass, time, current,
// temperature, amount, luminous intensity
std::array<int, 7> dimensions() const;
double scaleToSI() const;
QString symbol() const;

Unit operator*(const Unit& other) const;
Unit operator/(const Unit& other) const;
Unit pow(int exponent) const;
bool isCompatible(const Unit& other) const;

double convert(double value, const Unit& target) const;

// Common units pre-registered
static Unit volts();
static Unit millivolts();
static Unit seconds();
static Unit milliseconds();
// ... etc
};

### data/CoordinateArray.{h,cpp}

Stores coordinate values for one dimension. Either
regular (start + step + count, memory-efficient) or irregular
(explicit vector). Supports label-based lookup.

### data/TabularDataset.{h,cpp}

Concrete implementation for 1D tabular data (v1 DataFrame
replacement). rank() == 1, dimensions()[0] is "row", each column
is actually a separate Dataset sharing row dimension. Or:
columns are additional "dimensions" of rank 2 (row × column).

**Architect decides the exact mapping in the plan**. Two options:

(A) TabularDataset IS-A Dataset; columns are metadata
(B) A "tabular bundle" holds multiple rank-1 Datasets sharing
    the row dimension

Recommend (B) for cleaner separation: each column is a true
Dataset, and TabularBundle is a convenience grouping.

### data/Grid2D.{h,cpp}

2D scalar field. rank() == 2. Used in Phase 7 for heatmap,
contour. Two labeled coordinates (e.g. "x" and "y") plus
scalar value array.

### data/Volume3D.{h,cpp}

3D scalar field. rank() == 3. Foundation for Phase 8 3D
rendering. Memory manager aware (volumes are often large).

### Reactive layer

Every Dataset emits `changed()` when value data changes,
`coordinatesChanged()` when coordinate arrays change. Consumers
connect and refresh. Phase 7 plot engine will use these signals.

### Tests 6.1

- test_unit_parsing.cpp: "mV", "m/s^2", "kg*m^2/s^3" all parse
- test_unit_arithmetic.cpp: V = m*a checks dimensionally
- test_unit_conversion.cpp: mV → V, s → ms
- test_dimension.cpp: Dimension equality, mismatch detection
- test_coordinate_array.cpp: regular vs irregular, label lookup
- test_dataset_base.cpp: rank, shape, signals
- test_tabular_bundle.cpp: multi-column bundle, shared row dim
- test_grid2d.cpp: construction, valueAt, slice by label
- test_volume3d.cpp: same for 3D

## Phase 6.2 — DataFrame Migration

### Goal

Delete `lumen::data::DataFrame`. Replace with `TabularDataset`
or `TabularBundle` (per Architect decision). All 275 existing
tests pass unchanged behaviorally — same plot output, same CSV
loading, same Phase 4 workspace files loadable.

### Migration scope

- `src/lumen/data/DataFrame.{h,cpp}` → deleted
- All `DataFrame*` / `DataFrame&` references updated across
  data/, core/, plot/, ui/, io/
- Phase 2 `PlotScene` addSeries signatures updated
- Phase 4 `WorkspaceFile` load/save uses new Dataset API; v1
  workspace files must still load (migration on read)
- Phase 5 `LineSeries`, `ScatterSeries`, `BarSeries` constructors
  updated; they now reference Datasets by dimension index, not
  column index
- `DataTableDock` displays a TabularBundle; shows placeholder
  for Grid2D/Volume3D

### Backward compatibility

- v1 CSV files still open the same way
- v1 workspace files (.lumen.json) load by reading old
  column-index fields and mapping to new Dataset refs
- All 275 tests pass with zero behavioral change

### Tests 6.2

- test_dataframe_migration.cpp: byte-for-byte identical plot
  output before and after migration (render to QImage, compare)
- test_v1_workspace_load.cpp: load an actual Phase 4 workspace
  file, verify all edits restored
- Regression: full 275-test suite unchanged

## Phase 6.3 — File I/O and Memory Manager

### data/io/DatasetLoader.{h,cpp}

Loader interface. Each format has a concrete loader registered
at startup. Plugin-ready (for Phase 16).
class DatasetLoader {
public:
virtual ~DatasetLoader() = default;
virtual QStringList supportedExtensions() const = 0;
virtual std::unique_ptr<Dataset> load(const QString& path) = 0;
virtual bool canLoad(const QString& path) const;
};

### Concrete loaders

- `CsvLoader` (wraps Phase 1 CsvReader, produces TabularBundle)
- `Hdf5Loader` (HDF5 via hdf5 C++ library)
- `NetCDFLoader` (netcdf-c++)
- `ParquetLoader` (arrow C++)
- `ZarrLoader` (tensorstore or custom, for chunked access)
- `TiffStackLoader` (libtiff, for image stacks → Volume3D)
- `JsonLoader` (Qt's QJsonDocument, for structured tabular)
- `MatLoader` (Matio C library, for .mat files)
- `NumpyLoader` (custom .npy binary parser)

### data/MemoryManager.{h,cpp}

Hybrid memory strategy:
class MemoryManager {
public:
static MemoryManager& instance();
// Threshold: 100 MB
StorageMode decide(std::size_t estimatedBytes) const;

// LRU cache for chunked datasets
void registerChunkAccess(Dataset* ds, ChunkId chunk);
void evictIfNeeded();

std::size_t memoryBudgetBytes() const;  // configurable
std::size_t currentUsageBytes() const;

void setBudget(std::size_t bytes);  // user-settable
};

### Tests 6.3

- Round-trip tests for each loader: write synthetic data, load,
  compare
- test_hdf5_large_file.cpp: load 500 MB HDF5, verify chunked
  mode engaged, verify LRU evicts
- test_format_detection.cpp: correct loader picked by extension
- test_memory_budget.cpp: budget enforcement

## Phase 6.4 — UI Surface

### MainWindow additions

- File → Open: extended to recognize all 9 formats
- File → Open Sample → submenu with synthetic datasets:
  - "Sine 1D" (TabularBundle)
  - "Gaussian 2D" (Grid2D)
  - "Mandelbrot" (Grid2D)
  - "Volume Sphere" (Volume3D)
- Status bar: memory usage indicator ("342 MB / 4 GB")
- Settings: memory budget slider (default 4 GB, range 256 MB –
  system RAM)

### DataTableDock updates

- TabularBundle: existing display
- Grid2D: placeholder with dimension info
  ("Grid2D: 1024 × 1024 (float64), ready for Phase 7 heatmap")
- Volume3D: placeholder ("Volume3D: 256³ (float32), ready for
  Phase 8 rendering")

### No plot changes

Phase 7 adds the actual scalar field rendering. Phase 6 ends
with data loaded and visible in the table panel (for
TabularBundle) or as metadata (for Grid2D/Volume3D).

### Tests 6.4

- test_sample_menu.cpp: each synthetic sample loads
- test_open_hdf5_integration.cpp: HDF5 file opens, registers
  in DocumentRegistry, shows in DataTableDock

## ADRs

- **ADR-032** xarray-style Dataset model with labeled
  coordinates. Alternatives: tuple of 1D arrays (rejected,
  tabular-only), columnar DB (rejected, wrong abstraction).
- **ADR-033** Hybrid memory: in-memory < 100 MB, chunked LRU
  above. Alternatives: all-in-memory (rejected, no large files),
  always-chunked (rejected, overhead for small).
- **ADR-034** Physical units with dimensional analysis. Alts:
  strings as labels (rejected, no validation), full CAS
  (rejected, overkill).
- **ADR-035** Reactive Dataset signals from foundation.
  Alternatives: non-reactive + Phase 7 retrofit (rejected,
  churn).
- **ADR-036** DataFrame deletion, no legacy wrapper. Alternatives:
  deprecated wrapper (rejected, dual API).
- **ADR-037** Loader plugin interface. Alternatives: hardcoded
  switch (rejected, Phase 16 extensibility demands interface).

## Architecture updates

docs/architecture.md "Phase 6 additions" section:
- data/ module restructured: Dataset base, Dimension, Unit,
  CoordinateArray, TabularBundle, Grid2D, Volume3D
- data/io/ submodule: DatasetLoader + 9 concrete loaders
- data/MemoryManager: singleton, LRU, budget
- DataFrame removed from the architecture; TabularBundle is
  the replacement
- Reactive signals from Dataset propagate to future plot engine

## Acceptance criteria

Manual flow per sub-phase:

6.1: build clean, all 275 tests pass, new unit tests pass
6.2: open a v1 CSV, plot appears identical; open a Phase 4
     workspace, edits restored; all 275 tests pass unchanged
6.3: open an HDF5 file with 500 MB, app does not hang, memory
     usage stays within budget
6.4: File → Open Sample → Gaussian 2D; DataTableDock shows
     "Grid2D: 256 × 256 (float64), ready for Phase 7 heatmap"

Regression:
- [ ] Phase 2 pan/zoom/crosshair on v1 CSV
- [ ] Phase 3a/3b editing
- [ ] Phase 4 save/load workspace (v1 and v2 formats)
- [ ] Phase 4 export PNG/SVG/PDF
- [ ] Phase 5 line/scatter/bar

## Real-data exit criterion

Human opens at least one HDF5 or NetCDF file from real
scientific data, verifies it loads, verifies DocumentRegistry
reflects it, verifies memory stays within budget for a file
larger than 100 MB.

## Non-goals (deferred)

- Heatmap/contour rendering (Phase 7)
- Reactive plot updates on data change (Phase 7)
- 3D rendering (Phase 8)
- Domain-specific loaders: ND2, OME-TIFF, FITS, BIDS, EDF
  (later phases)
- User-defined plugins (Phase 16)
- Categorical units (not physical) — e.g. "count", "index"
  treated as dimensionless

## Risks

| Risk | Mitigation |
|---|---|
| DataFrame migration breaks v1 tests | Byte-identical render regression test; 275 tests hard gate |
| Unit parsing ambiguity ("m" = meter or milli?) | Explicit grammar; documented in ADR-034 |
| HDF5 C++ linking on Ubuntu 24.04 | Test in CI; fallback to C API if C++ bindings fragile |
| Memory manager LRU thrashing on marginal cases | Configurable budget; test with 1.5× budget load |
| Phase 4 workspace compat breaks | Dedicated test_v1_workspace_load with real fixture |

## Task breakdown

### Architect (S)
- phase-6-plan.md with 4 sub-phase structure
- ADR-032 through ADR-037
- Architecture update
- STATUS entry

### Backend (XL, split across sub-phases)
- 6.1: Dataset base, Unit, Dimension, CoordinateArray,
  TabularBundle, Grid2D, Volume3D
- 6.2: DataFrame deletion + migration of all callers
- 6.3: 9 loaders + MemoryManager

### Frontend (M)
- DataTableDock extension for Grid2D/Volume3D placeholders
- MainWindow File menu, sample menu, memory status bar

### QA (L)
- Unit tests per sub-phase
- Regression gate: 275 tests unchanged after 6.2
- Byte-identical render test
- Large file load tests

### Integration (S)
- Sub-phase merge windows, tag vphase-6

### Docs (S)
- README update
- data/CLAUDE.md rewrite
- data/io/CLAUDE.md new
- phase-6-review.md IN SAME COMMIT as closing STATUS entry

## Exit checklist

- [ ] Build clean, 0 warnings
- [ ] All 275 pre-existing tests pass unchanged after 6.2
- [ ] New Phase 6 unit tests pass (target 275 → 330+)
- [ ] 9 loaders functional
- [ ] v1 CSV and v1 workspace files load identically
- [ ] HDF5 500 MB file loads within memory budget
- [ ] Human verifies real scientific data file loads
- [ ] ADR-032 through ADR-037 committed
- [ ] phase-6-review.md committed IN SAME COMMIT as closing
      STATUS entry
- [ ] vphase-6 tag pushed
