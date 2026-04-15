#include "theme_fork.h"

namespace lumen::style {

bool forkTheme(ThemeRegistry& registry,
               const QString& sourceName,
               const QString& newName)
{
    if (!registry.hasTheme(sourceName))
        return false;

    if (registry.isBuiltin(newName))
        return false;

    Style source = registry.theme(sourceName);
    if (!registry.registerUserTheme(newName, source))
        return false;

    registry.setActiveTheme(newName);
    return true;
}

}  // namespace lumen::style
