# ADR-032: Dataset uses xarray-style n-d array with labeled coordinates

## Status
Accepted (Phase 6)

## Context
Lumen v1 uses DataFrame (vector of named typed columns) for all
data. This model cannot represent 2D grids (images, heatmaps), 3D
volumes, or any n-dimensional scientific data. Phase 6 needs a
universal data abstraction for Phases 7-16.

## Decision
Adopt the xarray model: an n-dimensional array with labeled
coordinates per dimension. Each Dataset has rank, shape, named
dimensions (each with a CoordinateArray and Unit), and a value unit.

Concrete implementations:
- Rank1Dataset: single 1D array (replaces Column)
- TabularBundle: groups multiple Rank1Datasets sharing a row dim
  (replaces DataFrame)
- Grid2D: 2D scalar field (for heatmaps, contours)
- Volume3D: 3D scalar field (for volume rendering)

Dataset is a QObject with changed() and coordinatesChanged()
signals for reactive updates.

## Consequences
- + Represents any scientific data dimensionality
- + Labeled coordinates enable label-based slicing and unit-aware
  axis generation
- + Physical units enable dimensional analysis (V = I * R)
- + Reactive signals prepare for Phase 7 live plot updates
- - Significant migration: every v1 caller must be updated
- - TabularBundle is not itself a Dataset, adding a level of
  indirection for tabular data

## Alternatives considered
- **Tuple of 1D arrays** (DataFrame-like): handles tabular only.
  Cannot represent 2D/3D data. Rejected.
- **Columnar database model** (DuckDB, Apache Arrow): powerful for
  queries but wrong abstraction for labeled scientific arrays.
  No coordinate/unit support. Rejected.
