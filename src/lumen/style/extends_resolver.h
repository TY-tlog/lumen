#pragma once

#include "theme_registry.h"
#include "types.h"

#include <QString>

namespace lumen::style {

/// Resolve an `extends` chain: user theme inherits from parent(s).
/// Token-level merge: user overrides only specified tokens, inherits rest.
/// Cycle detection: max depth 5.
/// Returns resolved Style or empty Style on error.
Style resolveExtends(const ThemeRegistry& registry,
                     const QString& themeName,
                     int maxDepth = 5);

/// Check if an extends chain has a cycle.
/// Returns empty string if OK, error message if cycle detected.
QString detectCycle(const ThemeRegistry& registry,
                    const QString& themeName,
                    int maxDepth = 5);

}  // namespace lumen::style
