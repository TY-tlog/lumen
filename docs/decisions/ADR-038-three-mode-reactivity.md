# ADR-038: Three-mode reactivity system (Static / DAG / Bidirectional)

## Status
Accepted (Phase 7)

## Context
Phase 6 built reactive signals into Dataset (changed(),
coordinatesChanged()). Phase 7 must decide how plots consume
these signals. Existing tools offer one fixed model: matplotlib
is static, Observable/Plotly are always-reactive, MATLAB is
static with linkdata. No tool offers reactivity as a first-class
user choice per plot.

## Decision
Three-mode system, user-selectable per plot via ReactivityModeWidget:

### Static mode (snapshot on bind)
On entering Static mode, ReactiveBinding deep-copies the bound
Dataset into an internal snapshot (unique_ptr<Dataset>). The
PlotItem reads from the snapshot, not the live Dataset.
Dataset::changed() is disconnected. Subsequent mutations to
the live Dataset do NOT affect the plot.

`invalidate()` re-snapshots from the live Dataset and re-renders.

Memory cost: 2× Dataset size while Static. Snapshot registered
with MemoryManager::trackAllocation() on create,
trackDeallocation() on destroy/refresh.

Use case: freeze a plot while exploring other data, compare
snapshots side by side.

### DAG mode (forward propagation)
ReactiveBinding connects Dataset::changed() → DependencyGraph
→ transitive downstream propagation → PlotItem::invalidate()
→ PlotCanvas::update(). PlotItem reads the live Dataset
directly (no copy). One-way: data changes flow to plot.

Use case: live acquisition, parameter sweep, filter pipeline.

### Bidirectional mode (plot edits push back to data)
DAG mode + write-back interceptor. When a plot property with a
data-mapped equivalent is edited via CommandBus (e.g., axis range
→ coordinate slice), ReactiveBinding intercepts and writes back
to the Dataset.

Generation counter in DependencyGraph prevents feedback: the
write-back increments the generation, and propagate() skips
notifications for the same generation.

Use case: exploratory analysis where selecting a range in the
plot filters the underlying data.

### API
```cpp
class ReactiveBinding : public QObject {
    void setMode(Mode m);
    Mode mode() const;
    void bindDataset(Dataset* ds);
    Dataset* dataSource() const;  // snapshot in Static, live in DAG/Bidir
    void invalidate();            // re-snapshot in Static, no-op in DAG
};
```

## Consequences
- + Novel capability: no existing tool offers this as a first-class
  choice
- + User picks the right model for their task
- + Static provides predictable freeze behavior with explicit refresh
- + DAG enables live data pipelines
- + Bidirectional enables exploratory workflows
- - Static mode doubles memory. Mitigated by MemoryManager budget.
- - Three modes to test and maintain
- - Bidirectional write-back is complex; generation counter adds
  bookkeeping

## Alternatives considered
- **Always-reactive** (like Observable): rejected; no escape hatch
  when the user wants a frozen snapshot. In electrophysiology
  analysis, comparing a filtered trace to the original requires
  one plot to be frozen.
- **Always-static** (like matplotlib): rejected; loses the power
  of live updates for acquisition and filter pipelines.
- **Mode per dataset, not per plot**: rejected; the plot is the
  user's mental unit. Two plots of the same dataset should be
  independently controllable (one frozen, one live).
