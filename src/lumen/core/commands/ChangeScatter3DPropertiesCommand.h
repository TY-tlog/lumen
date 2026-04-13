#pragma once

#include "core/Command.h"
#include "plot3d/Scatter3D.h"

#include <QColor>
#include <QString>

namespace lumen::plot3d {
class Scene3D;
}

namespace lumen::core::commands {

/// Command that changes the visual properties, name, and visibility of a
/// single Scatter3D item within a Scene3D.  Captures old values on
/// construction so that undo() can restore them.
class ChangeScatter3DPropertiesCommand : public Command {
public:
    ChangeScatter3DPropertiesCommand(
        plot3d::Scene3D* scene, int itemIndex,
        plot3d::Scatter3D::MarkerShape3D newMarkerShape,
        float newMarkerSize, QColor newColor, QString newName,
        bool newVisible);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    plot3d::Scene3D* scene_;
    int itemIndex_;

    plot3d::Scatter3D::MarkerShape3D oldMarkerShape_;
    plot3d::Scatter3D::MarkerShape3D newMarkerShape_;
    float oldMarkerSize_;
    float newMarkerSize_;
    QColor oldColor_;
    QColor newColor_;
    QString oldName_;
    QString newName_;
    bool oldVisible_;
    bool newVisible_;
};

}  // namespace lumen::core::commands
