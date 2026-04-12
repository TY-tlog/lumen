#include "ChangeContourPropertiesCommand.h"

#include "plot/ContourPlot.h"
#include "plot/PlotScene.h"

#include <cassert>

namespace lumen::core::commands {

ChangeContourPropertiesCommand::ChangeContourPropertiesCommand(
    plot::PlotScene* scene, std::size_t itemIndex,
    std::vector<double> newLevels, int newAutoLevelCount,
    QColor newLineColor, double newLineWidth, bool newLabelsVisible,
    QString newName, bool newVisible)
    : scene_(scene)
    , itemIndex_(itemIndex)
    , newLevels_(std::move(newLevels))
    , newAutoLevelCount_(newAutoLevelCount)
    , newLineColor_(std::move(newLineColor))
    , newLineWidth_(newLineWidth)
    , newLabelsVisible_(newLabelsVisible)
    , newName_(std::move(newName))
    , newVisible_(newVisible) {
    auto* item = scene_->itemAt(itemIndex_);
    auto* contour = dynamic_cast<plot::ContourPlot*>(item);
    assert(contour && "ChangeContourPropertiesCommand: item is not a ContourPlot");
    oldLevels_ = contour->levels();
    oldAutoLevelCount_ = 0;  // Will be determined by whether levels were auto
    oldLineColor_ = contour->lineColor();
    oldLineWidth_ = contour->lineWidth();
    oldLabelsVisible_ = contour->labelsVisible();
    oldName_ = contour->name();
    oldVisible_ = contour->isVisible();
}

void ChangeContourPropertiesCommand::execute() {
    auto* item = scene_->itemAt(itemIndex_);
    auto* contour = dynamic_cast<plot::ContourPlot*>(item);
    assert(contour);
    if (newAutoLevelCount_ > 0) {
        contour->setAutoLevels(newAutoLevelCount_);
    } else {
        contour->setLevels(newLevels_);
    }
    contour->setLineColor(newLineColor_);
    contour->setLineWidth(newLineWidth_);
    contour->setLabelsVisible(newLabelsVisible_);
    contour->setName(newName_);
    contour->setVisible(newVisible_);
}

void ChangeContourPropertiesCommand::undo() {
    auto* item = scene_->itemAt(itemIndex_);
    auto* contour = dynamic_cast<plot::ContourPlot*>(item);
    assert(contour);
    if (oldAutoLevelCount_ > 0) {
        contour->setAutoLevels(oldAutoLevelCount_);
    } else {
        contour->setLevels(oldLevels_);
    }
    contour->setLineColor(oldLineColor_);
    contour->setLineWidth(oldLineWidth_);
    contour->setLabelsVisible(oldLabelsVisible_);
    contour->setName(oldName_);
    contour->setVisible(oldVisible_);
}

QString ChangeContourPropertiesCommand::description() const {
    return QStringLiteral("Change contour properties of \"%1\"")
        .arg(newName_.isEmpty()
                 ? QStringLiteral("Item %1").arg(itemIndex_)
                 : newName_);
}

}  // namespace lumen::core::commands
