#include "ChangeHeatmapPropertiesCommand.h"

#include "plot/Heatmap.h"
#include "plot/PlotScene.h"

#include <cassert>

namespace lumen::core::commands {

ChangeHeatmapPropertiesCommand::ChangeHeatmapPropertiesCommand(
    plot::PlotScene* scene, std::size_t itemIndex,
    plot::Colormap newColormap, double newValueMin, double newValueMax,
    bool newAutoRange, plot::Heatmap::Interpolation newInterpolation,
    double newOpacity, QString newName, bool newVisible)
    : scene_(scene)
    , itemIndex_(itemIndex)
    , oldColormap_(plot::Colormap::builtin(plot::Colormap::Builtin::Viridis))
    , newColormap_(std::move(newColormap))
    , newValueMin_(newValueMin)
    , newValueMax_(newValueMax)
    , newAutoRange_(newAutoRange)
    , newInterpolation_(newInterpolation)
    , newOpacity_(newOpacity)
    , newName_(std::move(newName))
    , newVisible_(newVisible) {
    auto* item = scene_->itemAt(itemIndex_);
    auto* heatmap = dynamic_cast<plot::Heatmap*>(item);
    assert(heatmap && "ChangeHeatmapPropertiesCommand: item is not a Heatmap");
    oldColormap_ = heatmap->colormap();
    oldValueMin_ = heatmap->valueMin();
    oldValueMax_ = heatmap->valueMax();
    // Detect auto-range: if min==0 and max==1 initially, treat as auto.
    // We store the actual state based on the new flag pattern.
    oldAutoRange_ = false;  // Will be restored by value range
    oldInterpolation_ = heatmap->interpolation();
    oldOpacity_ = heatmap->opacity();
    oldName_ = heatmap->name();
    oldVisible_ = heatmap->isVisible();
}

void ChangeHeatmapPropertiesCommand::execute() {
    auto* item = scene_->itemAt(itemIndex_);
    auto* heatmap = dynamic_cast<plot::Heatmap*>(item);
    assert(heatmap);
    heatmap->setColormap(newColormap_);
    if (newAutoRange_) {
        heatmap->setAutoValueRange();
    } else {
        heatmap->setValueRange(newValueMin_, newValueMax_);
    }
    heatmap->setInterpolation(newInterpolation_);
    heatmap->setOpacity(newOpacity_);
    heatmap->setName(newName_);
    heatmap->setVisible(newVisible_);
}

void ChangeHeatmapPropertiesCommand::undo() {
    auto* item = scene_->itemAt(itemIndex_);
    auto* heatmap = dynamic_cast<plot::Heatmap*>(item);
    assert(heatmap);
    heatmap->setColormap(oldColormap_);
    if (oldAutoRange_) {
        heatmap->setAutoValueRange();
    } else {
        heatmap->setValueRange(oldValueMin_, oldValueMax_);
    }
    heatmap->setInterpolation(oldInterpolation_);
    heatmap->setOpacity(oldOpacity_);
    heatmap->setName(oldName_);
    heatmap->setVisible(oldVisible_);
}

QString ChangeHeatmapPropertiesCommand::description() const {
    return QStringLiteral("Change heatmap properties of \"%1\"")
        .arg(newName_.isEmpty()
                 ? QStringLiteral("Item %1").arg(itemIndex_)
                 : newName_);
}

}  // namespace lumen::core::commands
