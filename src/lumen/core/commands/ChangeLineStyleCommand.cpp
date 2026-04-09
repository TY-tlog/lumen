#include "ChangeLineStyleCommand.h"

#include "plot/LineSeries.h"
#include "plot/PlotScene.h"

namespace lumen::core::commands {

ChangeLineStyleCommand::ChangeLineStyleCommand(
    plot::PlotScene* scene, std::size_t seriesIndex,
    plot::PlotStyle newStyle, QString newName, bool newVisible)
    : scene_(scene)
    , seriesIndex_(seriesIndex)
    , newStyle_(std::move(newStyle))
    , newName_(std::move(newName))
    , newVisible_(newVisible) {
    // Capture old values from the current state of the series.
    auto& series = scene_->seriesAt(seriesIndex_);
    oldStyle_ = series.style();
    oldName_ = series.name();
    oldVisible_ = series.isVisible();
}

void ChangeLineStyleCommand::execute() {
    auto& series = scene_->seriesAt(seriesIndex_);
    series.setStyle(newStyle_);
    series.setName(newName_);
    series.setVisible(newVisible_);
}

void ChangeLineStyleCommand::undo() {
    auto& series = scene_->seriesAt(seriesIndex_);
    series.setStyle(oldStyle_);
    series.setName(oldName_);
    series.setVisible(oldVisible_);
}

QString ChangeLineStyleCommand::description() const {
    return QStringLiteral("Change line style of \"%1\"").arg(
        newName_.isEmpty() ? QStringLiteral("Series %1").arg(seriesIndex_)
                           : newName_);
}

}  // namespace lumen::core::commands
