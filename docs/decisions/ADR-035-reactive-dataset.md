# ADR-035: Reactive Dataset with changed() signals from foundation

## Status
Accepted (Phase 6)

## Context
Phase 7 introduces reactive plot updates: when data changes (e.g.,
a live acquisition appends rows), the plot redraws automatically.
This requires the data layer to notify consumers of changes.

The question is whether to build reactivity into Dataset from
Phase 6 or add it later in Phase 7.

## Decision
Dataset inherits QObject and emits `changed()` and
`coordinatesChanged()` signals from Phase 6 onward. Every
Dataset subclass (Rank1Dataset, Grid2D, Volume3D) has these
signals available. Phase 6 consumers (PlotCanvasDock,
DataTableDock) can connect but don't need to yet. Phase 7 plot
engine connects to auto-refresh.

## Consequences
- + No Phase 7 retrofit: signals are already wired
- + Phase 7 implementation is simpler: just connect changed() →
  repaint
- + TabularBundle can aggregate changed() from member Datasets
- - QObject inheritance adds overhead (~100 bytes per Dataset
  instance). Negligible for scientific datasets.
- - Dataset is non-copyable (QObject constraint). This matches
  Phase 5's PlotItem pattern and is correct: datasets are identity
  objects, not value objects.

## Alternatives considered
- **Non-reactive + Phase 7 retrofit**: build Dataset without
  QObject, add signals later. Rejected: the retrofit would touch
  every Dataset subclass, every constructor, and every caller that
  holds a Dataset. Doing it once in Phase 6 is cheaper.
- **Global EventBus for data changes**: publish data-change events
  through the existing EventBus instead of per-Dataset signals.
  Rejected: EventBus is cross-module (document lifecycle), not
  per-dataset. A dataset's changed() signal is a direct
  notification to its consumers, not a broadcast.
