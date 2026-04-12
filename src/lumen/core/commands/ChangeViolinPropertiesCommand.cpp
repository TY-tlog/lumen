#include "ChangeViolinPropertiesCommand.h"

#include "plot/PlotScene.h"
#include "plot/ViolinSeries.h"

#include <cassert>

namespace lumen::core::commands {

ChangeViolinPropertiesCommand::ChangeViolinPropertiesCommand(
    plot::PlotScene* scene, std::size_t itemIndex, double newBandwidth,
    bool newAutoKde, bool newSplit, QColor newFillColor, QString newName,
    bool newVisible)
    : scene_(scene)
    , itemIndex_(itemIndex)
    , newBandwidth_(newBandwidth)
    , newAutoKde_(newAutoKde)
    , newSplit_(newSplit)
    , newFillColor_(std::move(newFillColor))
    , newName_(std::move(newName))
    , newVisible_(newVisible) {
    auto* item = scene_->itemAt(itemIndex_);
    auto* violin = dynamic_cast<plot::ViolinSeries*>(item);
    assert(violin && "ChangeViolinPropertiesCommand: item is not a ViolinSeries");
    oldBandwidth_ = violin->kdeBandwidth();
    oldAutoKde_ = violin->kdeBandwidthAuto();
    oldSplit_ = violin->split();
    oldFillColor_ = violin->fillColor();
    oldName_ = violin->name();
    oldVisible_ = violin->isVisible();
}

void ChangeViolinPropertiesCommand::execute() {
    auto* item = scene_->itemAt(itemIndex_);
    auto* violin = dynamic_cast<plot::ViolinSeries*>(item);
    assert(violin);
    violin->setKdeBandwidthAuto(newAutoKde_);
    if (!newAutoKde_) {
        violin->setKdeBandwidth(newBandwidth_);
    }
    violin->setSplit(newSplit_);
    violin->setFillColor(newFillColor_);
    violin->setName(newName_);
    violin->setVisible(newVisible_);
}

void ChangeViolinPropertiesCommand::undo() {
    auto* item = scene_->itemAt(itemIndex_);
    auto* violin = dynamic_cast<plot::ViolinSeries*>(item);
    assert(violin);
    violin->setKdeBandwidthAuto(oldAutoKde_);
    if (!oldAutoKde_) {
        violin->setKdeBandwidth(oldBandwidth_);
    }
    violin->setSplit(oldSplit_);
    violin->setFillColor(oldFillColor_);
    violin->setName(oldName_);
    violin->setVisible(oldVisible_);
}

QString ChangeViolinPropertiesCommand::description() const {
    return QStringLiteral("Change violin properties of \"%1\"")
        .arg(newName_.isEmpty()
                 ? QStringLiteral("Item %1").arg(itemIndex_)
                 : newName_);
}

}  // namespace lumen::core::commands
