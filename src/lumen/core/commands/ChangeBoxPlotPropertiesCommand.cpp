#include "ChangeBoxPlotPropertiesCommand.h"

#include "plot/BoxPlotSeries.h"
#include "plot/PlotScene.h"

#include <cassert>

namespace lumen::core::commands {

ChangeBoxPlotPropertiesCommand::ChangeBoxPlotPropertiesCommand(
    plot::PlotScene* scene, std::size_t itemIndex,
    plot::BoxPlotSeries::WhiskerRule newWhiskerRule, bool newNotched,
    bool newOutliersVisible, QColor newFillColor, QString newName,
    bool newVisible)
    : scene_(scene)
    , itemIndex_(itemIndex)
    , newWhiskerRule_(newWhiskerRule)
    , newNotched_(newNotched)
    , newOutliersVisible_(newOutliersVisible)
    , newFillColor_(std::move(newFillColor))
    , newName_(std::move(newName))
    , newVisible_(newVisible) {
    auto* item = scene_->itemAt(itemIndex_);
    auto* box = dynamic_cast<plot::BoxPlotSeries*>(item);
    assert(box && "ChangeBoxPlotPropertiesCommand: item is not a BoxPlotSeries");
    oldWhiskerRule_ = box->whiskerRule();
    oldNotched_ = box->notched();
    oldOutliersVisible_ = box->outliersVisible();
    oldFillColor_ = box->fillColor();
    oldName_ = box->name();
    oldVisible_ = box->isVisible();
}

void ChangeBoxPlotPropertiesCommand::execute() {
    auto* item = scene_->itemAt(itemIndex_);
    auto* box = dynamic_cast<plot::BoxPlotSeries*>(item);
    assert(box);
    box->setWhiskerRule(newWhiskerRule_);
    box->setNotched(newNotched_);
    box->setOutliersVisible(newOutliersVisible_);
    box->setFillColor(newFillColor_);
    box->setName(newName_);
    box->setVisible(newVisible_);
}

void ChangeBoxPlotPropertiesCommand::undo() {
    auto* item = scene_->itemAt(itemIndex_);
    auto* box = dynamic_cast<plot::BoxPlotSeries*>(item);
    assert(box);
    box->setWhiskerRule(oldWhiskerRule_);
    box->setNotched(oldNotched_);
    box->setOutliersVisible(oldOutliersVisible_);
    box->setFillColor(oldFillColor_);
    box->setName(oldName_);
    box->setVisible(oldVisible_);
}

QString ChangeBoxPlotPropertiesCommand::description() const {
    return QStringLiteral("Change box plot properties of \"%1\"")
        .arg(newName_.isEmpty()
                 ? QStringLiteral("Item %1").arg(itemIndex_)
                 : newName_);
}

}  // namespace lumen::core::commands
