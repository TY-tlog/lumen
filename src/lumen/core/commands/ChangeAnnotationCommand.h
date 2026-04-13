#pragma once

#include "core/Command.h"

#include <QJsonObject>
#include <QString>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core::commands {

/// Command that changes properties of an Annotation within a PlotScene.
///
/// Uses JSON snapshots for generic undo/redo: captures the full
/// annotation state before and after the edit. This avoids needing
/// a separate command class per annotation type.
class ChangeAnnotationCommand : public Command {
public:
    /// @param scene The PlotScene containing the annotation.
    /// @param annotationId The annotation's ID within AnnotationLayer.
    /// @param newState The new JSON state to apply (from toJson()).
    ChangeAnnotationCommand(plot::PlotScene* scene, int annotationId,
                            QJsonObject newState);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    void applyState(const QJsonObject& state);

    plot::PlotScene* scene_;
    int annotationId_;
    QJsonObject oldState_;
    QJsonObject newState_;
};

}  // namespace lumen::core::commands
