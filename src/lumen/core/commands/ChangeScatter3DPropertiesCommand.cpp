#include "ChangeScatter3DPropertiesCommand.h"

#include "plot3d/Scene3D.h"
#include "plot3d/Scatter3D.h"

#include <cassert>

namespace lumen::core::commands {

ChangeScatter3DPropertiesCommand::ChangeScatter3DPropertiesCommand(
    plot3d::Scene3D* scene, int itemIndex,
    plot3d::Scatter3D::MarkerShape3D newMarkerShape,
    float newMarkerSize, QColor newColor, QString newName,
    bool newVisible)
    : scene_(scene)
    , itemIndex_(itemIndex)
    , newMarkerShape_(newMarkerShape)
    , newMarkerSize_(newMarkerSize)
    , newColor_(std::move(newColor))
    , newName_(std::move(newName))
    , newVisible_(newVisible)
{
    auto* item = scene_->itemAt(itemIndex_);
    auto* scatter = dynamic_cast<plot3d::Scatter3D*>(item);
    assert(scatter && "ChangeScatter3DPropertiesCommand: item is not a Scatter3D");
    oldMarkerShape_ = scatter->markerShape();
    oldMarkerSize_ = scatter->markerSize();
    oldColor_ = scatter->color();
    oldName_ = scatter->name();
    oldVisible_ = scatter->isVisible();
}

void ChangeScatter3DPropertiesCommand::execute()
{
    auto* item = scene_->itemAt(itemIndex_);
    auto* scatter = dynamic_cast<plot3d::Scatter3D*>(item);
    assert(scatter);
    scatter->setMarkerShape(newMarkerShape_);
    scatter->setMarkerSize(newMarkerSize_);
    scatter->setColor(newColor_);
    scatter->setName(newName_);
    scatter->setVisible(newVisible_);
}

void ChangeScatter3DPropertiesCommand::undo()
{
    auto* item = scene_->itemAt(itemIndex_);
    auto* scatter = dynamic_cast<plot3d::Scatter3D*>(item);
    assert(scatter);
    scatter->setMarkerShape(oldMarkerShape_);
    scatter->setMarkerSize(oldMarkerSize_);
    scatter->setColor(oldColor_);
    scatter->setName(oldName_);
    scatter->setVisible(oldVisible_);
}

QString ChangeScatter3DPropertiesCommand::description() const
{
    return QStringLiteral("Change 3D scatter properties of \"%1\"")
        .arg(newName_.isEmpty()
                 ? QStringLiteral("Item %1").arg(itemIndex_)
                 : newName_);
}

}  // namespace lumen::core::commands
