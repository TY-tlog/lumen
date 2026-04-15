#pragma once

#include "core/Command.h"
#include "style/cascade.h"
#include "style/types.h"

#include <QJsonObject>
#include <QString>

namespace lumen::style {
class ThemeRegistry;
}

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core::commands {

/// Command that edits a style property at a specific cascade level.
/// Preserves the cascade level in the undo stack so undo restores
/// the correct level, not just the value.
class StyleEditCommand : public Command {
public:
    StyleEditCommand(style::CascadeLevel level,
                     const QString& propertyPath,
                     const QJsonObject& oldValue,
                     const QJsonObject& newValue,
                     plot::PlotScene* scene,
                     style::ThemeRegistry* registry,
                     const QString& themeName);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    style::CascadeLevel level_;
    QString propertyPath_;
    QJsonObject oldValue_;
    QJsonObject newValue_;
    plot::PlotScene* scene_;
    style::ThemeRegistry* registry_;
    QString themeName_;
};

}  // namespace lumen::core::commands
