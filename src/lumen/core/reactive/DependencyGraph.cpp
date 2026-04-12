#include "DependencyGraph.h"

#include <algorithm>
#include <stdexcept>

namespace lumen::reactive {

DependencyGraph::DependencyGraph(QObject* parent)
    : QObject(parent)
{
}

void DependencyGraph::addNode(data::Dataset* ds)
{
    if (ds == nullptr) {
        return;
    }
    // Ensure the node exists in the adjacency map (even with no edges).
    if (adjacency_.find(ds) == adjacency_.end()) {
        adjacency_[ds] = {};
    }
}

void DependencyGraph::addDerivation(data::Dataset* source, data::Dataset* derived,
                                    std::function<void()> updateFn)
{
    if (source == nullptr || derived == nullptr) {
        throw std::invalid_argument("DependencyGraph::addDerivation: null pointer");
    }
    if (source == derived) {
        throw std::invalid_argument("DependencyGraph::addDerivation: self-loop");
    }

    // Ensure both nodes exist.
    addNode(source);
    addNode(derived);

    // Cycle detection: check if source is reachable from derived.
    // If so, adding derived -> ... -> source -> derived would create a cycle.
    if (wouldCreateCycle(derived, source)) {
        throw std::invalid_argument(
            "DependencyGraph::addDerivation: would create a cycle");
    }

    adjacency_[source].push_back({derived, std::move(updateFn)});
}

void DependencyGraph::propagate(data::Dataset* changed)
{
    if (changed == nullptr) {
        return;
    }

    ++generation_;

    // BFS through downstream nodes.
    std::vector<data::Dataset*> queue;
    std::vector<data::Dataset*> visited;

    // Start with direct dependents of the changed node.
    auto it = adjacency_.find(changed);
    if (it != adjacency_.end()) {
        for (const auto& edge : it->second) {
            queue.push_back(edge.derived);
        }
    }

    // Also emit for the changed node itself.
    emit nodeUpdated(changed);

    std::size_t idx = 0;
    while (idx < queue.size()) {
        data::Dataset* current = queue[idx++];

        // Skip if already visited.
        if (std::find(visited.begin(), visited.end(), current) != visited.end()) {
            continue;
        }
        visited.push_back(current);

        // Find and invoke the update function for this edge.
        // We need to find which parent produced this node.
        // Walk all adjacency entries to find edges pointing to 'current'.
        for (const auto& [parent, edges] : adjacency_) {
            for (const auto& edge : edges) {
                if (edge.derived == current) {
                    if (edge.updateFn) {
                        edge.updateFn();
                    }
                    break; // Only invoke once per derived node
                }
            }
        }

        emit nodeUpdated(current);

        // Enqueue downstream of current.
        auto it2 = adjacency_.find(current);
        if (it2 != adjacency_.end()) {
            for (const auto& edge : it2->second) {
                if (std::find(visited.begin(), visited.end(), edge.derived) == visited.end()) {
                    queue.push_back(edge.derived);
                }
            }
        }
    }
}

std::vector<data::Dataset*> DependencyGraph::downstream(data::Dataset* ds) const
{
    std::vector<data::Dataset*> result;
    std::vector<data::Dataset*> visited;
    collectDownstream(ds, result, visited);
    return result;
}

uint64_t DependencyGraph::generation() const
{
    return generation_;
}

bool DependencyGraph::wouldCreateCycle(data::Dataset* start, data::Dataset* target) const
{
    // DFS from 'start' to see if 'target' is reachable.
    std::vector<data::Dataset*> stack;
    std::vector<data::Dataset*> visited;

    stack.push_back(start);

    while (!stack.empty()) {
        data::Dataset* current = stack.back();
        stack.pop_back();

        if (current == target) {
            return true;
        }

        if (std::find(visited.begin(), visited.end(), current) != visited.end()) {
            continue;
        }
        visited.push_back(current);

        auto it = adjacency_.find(current);
        if (it != adjacency_.end()) {
            for (const auto& edge : it->second) {
                stack.push_back(edge.derived);
            }
        }
    }

    return false;
}

void DependencyGraph::collectDownstream(data::Dataset* ds,
                                        std::vector<data::Dataset*>& result,
                                        std::vector<data::Dataset*>& visited) const
{
    if (ds == nullptr) {
        return;
    }

    auto it = adjacency_.find(ds);
    if (it == adjacency_.end()) {
        return;
    }

    for (const auto& edge : it->second) {
        if (std::find(visited.begin(), visited.end(), edge.derived) != visited.end()) {
            continue;
        }
        visited.push_back(edge.derived);
        result.push_back(edge.derived);
        collectDownstream(edge.derived, result, visited);
    }
}

} // namespace lumen::reactive
