#include "ChangeScatterPropertiesCommand.h"

#include "plot/PlotScene.h"
#include "plot/ScatterSeries.h"

#include <cassert>

namespace lumen::core::commands {

ChangeScatterPropertiesCommand::ChangeScatterPropertiesCommand(
    plot::PlotScene* scene, std::size_t seriesIndex, QColor newColor,
    plot::MarkerShape newMarkerShape, int newMarkerSize, bool newFilled,
    QString newName, bool newVisible)
    : scene_(scene)
    , seriesIndex_(seriesIndex)
    , newColor_(std::move(newColor))
    , newMarkerShape_(newMarkerShape)
    , newMarkerSize_(newMarkerSize)
    , newFilled_(newFilled)
    , newName_(std::move(newName))
    , newVisible_(newVisible) {
    auto* item = scene_->itemAt(seriesIndex_);
    auto* scatter = dynamic_cast<plot::ScatterSeries*>(item);
    assert(scatter && "ChangeScatterPropertiesCommand: item is not a ScatterSeries");
    oldColor_ = scatter->color();
    oldMarkerShape_ = scatter->markerShape();
    oldMarkerSize_ = scatter->markerSize();
    oldFilled_ = scatter->filled();
    oldName_ = scatter->name();
    oldVisible_ = scatter->isVisible();
}

void ChangeScatterPropertiesCommand::execute() {
    auto* item = scene_->itemAt(seriesIndex_);
    auto* scatter = dynamic_cast<plot::ScatterSeries*>(item);
    assert(scatter);
    scatter->setColor(newColor_);
    scatter->setMarkerShape(newMarkerShape_);
    scatter->setMarkerSize(newMarkerSize_);
    scatter->setFilled(newFilled_);
    scatter->setName(newName_);
    scatter->setVisible(newVisible_);
}

void ChangeScatterPropertiesCommand::undo() {
    auto* item = scene_->itemAt(seriesIndex_);
    auto* scatter = dynamic_cast<plot::ScatterSeries*>(item);
    assert(scatter);
    scatter->setColor(oldColor_);
    scatter->setMarkerShape(oldMarkerShape_);
    scatter->setMarkerSize(oldMarkerSize_);
    scatter->setFilled(oldFilled_);
    scatter->setName(oldName_);
    scatter->setVisible(oldVisible_);
}

QString ChangeScatterPropertiesCommand::description() const {
    return QStringLiteral("Change scatter properties of \"%1\"")
        .arg(newName_.isEmpty()
                 ? QStringLiteral("Series %1").arg(seriesIndex_)
                 : newName_);
}

}  // namespace lumen::core::commands
