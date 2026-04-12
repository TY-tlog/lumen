#pragma once

#include "DependencyGraph.h"
#include "ReactiveMode.h"

#include <data/Dataset.h>
#include <plot/PlotItem.h>

#include <QObject>

#include <cstdint>
#include <memory>

namespace lumen::reactive {

/// Per-plot binding that controls how the plot consumes Dataset changes.
///
/// Three modes (ADR-038):
/// - Static:  deep-copy snapshot; Dataset::changed() is disconnected.
/// - DAG:     live Dataset; changed() triggers invalidate/repaint.
/// - Bidirectional: DAG + write-back; generation counter prevents feedback.
///
/// Owns the snapshot in Static mode and tracks its memory with MemoryManager.
class ReactiveBinding : public QObject {
    Q_OBJECT

public:
    /// Construct a binding for the given plot item, starting in the given mode.
    ReactiveBinding(plot::PlotItem* plot, Mode mode, QObject* parent = nullptr);
    ~ReactiveBinding() override;

    /// Switch to a new reactivity mode.
    void setMode(Mode m);

    /// Current mode.
    [[nodiscard]] Mode mode() const;

    /// Bind a live Dataset to this plot.
    void bindDataset(data::Dataset* ds);

    /// Return the data source: snapshot in Static mode, live Dataset otherwise.
    [[nodiscard]] data::Dataset* dataSource() const;

    /// In Static mode: re-snapshot from the live Dataset and re-register memory.
    /// In DAG/Bidirectional: request repaint.
    void invalidate();

    /// Set the DependencyGraph (optional, used for DAG/Bidirectional modes).
    void setGraph(DependencyGraph* graph);

signals:
    void modeChanged(Mode m);
    void repaintRequested();

private:
    void enterStaticMode();
    void enterDagMode();
    void enterBidirectionalMode();
    void onDatasetChanged();
    void destroySnapshot();
    void createSnapshot();

    plot::PlotItem* plot_;
    data::Dataset* liveDataset_ = nullptr;
    std::unique_ptr<data::Dataset> snapshot_;
    Mode mode_;
    DependencyGraph* graph_ = nullptr;
    uint64_t lastGeneration_ = 0;
    QMetaObject::Connection changedConnection_;
};

} // namespace lumen::reactive
