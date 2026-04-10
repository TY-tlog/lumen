#include "ChangeBarPropertiesCommand.h"

#include "plot/BarSeries.h"
#include "plot/PlotScene.h"

#include <cassert>

namespace lumen::core::commands {

ChangeBarPropertiesCommand::ChangeBarPropertiesCommand(
    plot::PlotScene* scene, std::size_t seriesIndex, QColor newFillColor,
    QColor newOutlineColor, double newBarWidth, QString newName,
    bool newVisible)
    : scene_(scene)
    , seriesIndex_(seriesIndex)
    , newFillColor_(std::move(newFillColor))
    , newOutlineColor_(std::move(newOutlineColor))
    , newBarWidth_(newBarWidth)
    , newName_(std::move(newName))
    , newVisible_(newVisible) {
    auto* item = scene_->itemAt(seriesIndex_);
    auto* bar = dynamic_cast<plot::BarSeries*>(item);
    assert(bar && "ChangeBarPropertiesCommand: item is not a BarSeries");
    oldFillColor_ = bar->fillColor();
    oldOutlineColor_ = bar->outlineColor();
    oldBarWidth_ = bar->barWidth();
    oldName_ = bar->name();
    oldVisible_ = bar->isVisible();
}

void ChangeBarPropertiesCommand::execute() {
    auto* item = scene_->itemAt(seriesIndex_);
    auto* bar = dynamic_cast<plot::BarSeries*>(item);
    assert(bar);
    bar->setFillColor(newFillColor_);
    bar->setOutlineColor(newOutlineColor_);
    bar->setBarWidth(newBarWidth_);
    bar->setName(newName_);
    bar->setVisible(newVisible_);
}

void ChangeBarPropertiesCommand::undo() {
    auto* item = scene_->itemAt(seriesIndex_);
    auto* bar = dynamic_cast<plot::BarSeries*>(item);
    assert(bar);
    bar->setFillColor(oldFillColor_);
    bar->setOutlineColor(oldOutlineColor_);
    bar->setBarWidth(oldBarWidth_);
    bar->setName(oldName_);
    bar->setVisible(oldVisible_);
}

QString ChangeBarPropertiesCommand::description() const {
    return QStringLiteral("Change bar properties of \"%1\"")
        .arg(newName_.isEmpty()
                 ? QStringLiteral("Series %1").arg(seriesIndex_)
                 : newName_);
}

}  // namespace lumen::core::commands
