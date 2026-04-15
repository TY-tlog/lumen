#include "StyleEditCommand.h"

#include "style/theme_registry.h"

namespace lumen::core::commands {

StyleEditCommand::StyleEditCommand(
    style::CascadeLevel level,
    const QString& propertyPath,
    const QJsonObject& oldValue,
    const QJsonObject& newValue,
    plot::PlotScene* scene,
    style::ThemeRegistry* registry,
    const QString& themeName)
    : level_(level)
    , propertyPath_(propertyPath)
    , oldValue_(oldValue)
    , newValue_(newValue)
    , scene_(scene)
    , registry_(registry)
    , themeName_(themeName)
{
}

void StyleEditCommand::execute()
{
    // Apply new value at the specified cascade level.
    // For Theme level: modify the theme in the registry.
    // For PlotInstance/ElementOverride: stored on PlotScene (future integration).
    Q_UNUSED(scene_)
    Q_UNUSED(registry_)
    // Concrete application logic depends on PlotScene style storage,
    // which is wired in Phase 10.3 UI integration.
}

void StyleEditCommand::undo()
{
    // Restore old value at the same cascade level.
    Q_UNUSED(scene_)
    Q_UNUSED(registry_)
}

QString StyleEditCommand::description() const
{
    QString levelStr;
    switch (level_) {
    case style::CascadeLevel::Theme:
        levelStr = QStringLiteral("theme (%1)").arg(themeName_);
        break;
    case style::CascadeLevel::Preset:
        levelStr = QStringLiteral("preset");
        break;
    case style::CascadeLevel::PlotInstance:
        levelStr = QStringLiteral("plot");
        break;
    case style::CascadeLevel::ElementOverride:
        levelStr = QStringLiteral("element");
        break;
    }
    return QStringLiteral("Edit %1 at %2 level").arg(propertyPath_, levelStr);
}

}  // namespace lumen::core::commands
