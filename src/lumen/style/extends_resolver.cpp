#include "extends_resolver.h"
#include "cascade.h"

#include <QSet>

namespace lumen::style {

Style resolveExtends(const ThemeRegistry& registry,
                     const QString& themeName,
                     int /*maxDepth*/)
{
    // For now, extends resolution is flat: theme inherits directly
    // from its named parent. The cascade resolver handles the merge.
    // Full chain walking requires the `extends` field to be stored
    // alongside the Style in the registry (Phase 10.4+ enhancement).

    if (!registry.hasTheme(themeName))
        return {};

    return registry.theme(themeName);
}

QString detectCycle(const ThemeRegistry& registry,
                    const QString& themeName,
                    int maxDepth)
{
    Q_UNUSED(registry)

    // With flat resolution (no stored extends chain), cycles are
    // impossible. This function is scaffolded for when the registry
    // stores extends metadata.

    if (themeName.isEmpty())
        return QStringLiteral("Empty theme name");

    if (maxDepth <= 0)
        return QStringLiteral("Extends chain depth exceeded");

    return {};
}

}  // namespace lumen::style
