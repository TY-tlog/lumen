#pragma once

#include "theme_registry.h"
#include "types.h"

#include <QString>

namespace lumen::style {

/// Fork a built-in theme into a user theme.
///
/// Copies the built-in theme's Style, registers it under the new
/// name, and sets it as the active theme.
/// Returns true on success, false if source doesn't exist or
/// newName conflicts with a built-in.
bool forkTheme(ThemeRegistry& registry,
               const QString& sourceName,
               const QString& newName);

}  // namespace lumen::style
