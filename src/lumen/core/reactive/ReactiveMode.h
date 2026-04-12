#pragma once

namespace lumen::reactive {

/// Three-mode reactivity system (ADR-038).
///
/// Static:        snapshot on bind; live Dataset mutations do not affect the plot.
/// DAG:           forward propagation; Dataset::changed() cascades to plot.
/// Bidirectional: DAG + write-back; plot edits push back to data.
enum class Mode { Static, DAG, Bidirectional };

} // namespace lumen::reactive
