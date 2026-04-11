#include "ReactiveBinding.h"

#include <data/MemoryManager.h>

namespace lumen::reactive {

ReactiveBinding::ReactiveBinding(plot::PlotItem* plot, Mode mode, QObject* parent)
    : QObject(parent)
    , plot_(plot)
    , mode_(mode)
{
}

ReactiveBinding::~ReactiveBinding()
{
    destroySnapshot();
}

void ReactiveBinding::setMode(Mode m)
{
    if (m == mode_) {
        return;
    }

    // Tear down old mode.
    if (changedConnection_) {
        disconnect(changedConnection_);
        changedConnection_ = {};
    }
    destroySnapshot();

    mode_ = m;

    // Enter new mode.
    switch (mode_) {
    case Mode::Static:
        enterStaticMode();
        break;
    case Mode::DAG:
        enterDagMode();
        break;
    case Mode::Bidirectional:
        enterBidirectionalMode();
        break;
    }

    emit modeChanged(mode_);
}

Mode ReactiveBinding::mode() const
{
    return mode_;
}

void ReactiveBinding::bindDataset(data::Dataset* ds)
{
    // Disconnect from previous dataset.
    if (changedConnection_) {
        disconnect(changedConnection_);
        changedConnection_ = {};
    }
    destroySnapshot();

    liveDataset_ = ds;

    // Re-enter the current mode with the new dataset.
    switch (mode_) {
    case Mode::Static:
        enterStaticMode();
        break;
    case Mode::DAG:
        enterDagMode();
        break;
    case Mode::Bidirectional:
        enterBidirectionalMode();
        break;
    }
}

data::Dataset* ReactiveBinding::dataSource() const
{
    if (mode_ == Mode::Static && snapshot_) {
        return snapshot_.get();
    }
    return liveDataset_;
}

void ReactiveBinding::invalidate()
{
    if (mode_ == Mode::Static) {
        // Re-snapshot from live dataset.
        destroySnapshot();
        createSnapshot();
    }
    emit repaintRequested();
}

void ReactiveBinding::setGraph(DependencyGraph* graph)
{
    graph_ = graph;
}

void ReactiveBinding::enterStaticMode()
{
    // Disconnect from Dataset::changed().
    if (changedConnection_) {
        disconnect(changedConnection_);
        changedConnection_ = {};
    }

    // Deep-copy the bound Dataset into snapshot.
    createSnapshot();
}

void ReactiveBinding::enterDagMode()
{
    // Destroy any existing snapshot.
    destroySnapshot();

    // Connect Dataset::changed() -> onDatasetChanged().
    if (liveDataset_ != nullptr) {
        changedConnection_ = connect(
            liveDataset_, &data::Dataset::changed,
            this, &ReactiveBinding::onDatasetChanged);
    }
}

void ReactiveBinding::enterBidirectionalMode()
{
    // Same as DAG + generation counter for feedback prevention.
    destroySnapshot();

    if (liveDataset_ != nullptr) {
        changedConnection_ = connect(
            liveDataset_, &data::Dataset::changed,
            this, &ReactiveBinding::onDatasetChanged);
    }

    if (graph_ != nullptr) {
        lastGeneration_ = graph_->generation();
    }
}

void ReactiveBinding::onDatasetChanged()
{
    // Feedback loop prevention for Bidirectional mode.
    if (mode_ == Mode::Bidirectional && graph_ != nullptr) {
        if (graph_->generation() == lastGeneration_) {
            return; // Suppress: this change was caused by our own write-back.
        }
        lastGeneration_ = graph_->generation();
    }

    // Request repaint.
    if (plot_ != nullptr) {
        plot_->invalidate();
    }
    emit repaintRequested();
}

void ReactiveBinding::destroySnapshot()
{
    if (snapshot_) {
        auto bytes = snapshot_->sizeBytes();
        snapshot_.reset();
        data::MemoryManager::instance().trackDeallocation(bytes);
    }
}

void ReactiveBinding::createSnapshot()
{
    if (liveDataset_ == nullptr) {
        return;
    }
    snapshot_ = liveDataset_->clone();
    if (snapshot_) {
        data::MemoryManager::instance().trackAllocation(snapshot_->sizeBytes());
    }
}

} // namespace lumen::reactive
