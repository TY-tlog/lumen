#pragma once

#include <data/Dataset.h>

#include <QObject>

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

namespace lumen::reactive {

/// Directed acyclic graph tracking derivation relationships between Datasets.
///
/// Each edge represents "source was used to derive target via updateFn".
/// propagate(changed) walks the DAG forward, invoking update functions
/// and emitting nodeUpdated for each affected Dataset. A generation counter
/// is incremented on each propagate call for feedback-loop prevention
/// in Bidirectional mode (ADR-038).
class DependencyGraph : public QObject {
    Q_OBJECT

public:
    explicit DependencyGraph(QObject* parent = nullptr);

    /// Register a Dataset as a node in the graph.
    void addNode(data::Dataset* ds);

    /// Add a derivation edge: source -> derived, with an update function.
    /// Throws std::invalid_argument if the edge would create a cycle.
    void addDerivation(data::Dataset* source, data::Dataset* derived,
                       std::function<void()> updateFn);

    /// Propagate a change from the given Dataset through its downstream
    /// dependents, invoking update functions along the way.
    /// Increments the generation counter.
    void propagate(data::Dataset* changed);

    /// Return all transitive downstream dependents of the given Dataset.
    [[nodiscard]] std::vector<data::Dataset*> downstream(data::Dataset* ds) const;

    /// Current generation counter value (incremented on each propagate call).
    [[nodiscard]] uint64_t generation() const;

signals:
    /// Emitted for each Dataset that was updated during propagation.
    void nodeUpdated(data::Dataset* ds);

private:
    struct Edge {
        data::Dataset* derived;
        std::function<void()> updateFn;
    };

    /// DFS cycle check: returns true if 'target' is reachable from 'start'.
    [[nodiscard]] bool wouldCreateCycle(data::Dataset* start, data::Dataset* target) const;

    /// Collect transitive downstream nodes via DFS.
    void collectDownstream(data::Dataset* ds, std::vector<data::Dataset*>& result,
                           std::vector<data::Dataset*>& visited) const;

    std::unordered_map<data::Dataset*, std::vector<Edge>> adjacency_;
    uint64_t generation_ = 0;
};

} // namespace lumen::reactive
