#include "ChangeHistogramPropertiesCommand.h"

#include "plot/HistogramSeries.h"
#include "plot/PlotScene.h"

#include <cassert>

namespace lumen::core::commands {

ChangeHistogramPropertiesCommand::ChangeHistogramPropertiesCommand(
    plot::PlotScene* scene, std::size_t itemIndex, int newBinCount,
    plot::HistogramSeries::BinRule newBinRule,
    plot::HistogramSeries::Normalization newNormalization,
    QColor newFillColor, QString newName, bool newVisible)
    : scene_(scene)
    , itemIndex_(itemIndex)
    , newBinCount_(newBinCount)
    , newBinRule_(newBinRule)
    , newNormalization_(newNormalization)
    , newFillColor_(std::move(newFillColor))
    , newName_(std::move(newName))
    , newVisible_(newVisible) {
    auto* item = scene_->itemAt(itemIndex_);
    auto* hist = dynamic_cast<plot::HistogramSeries*>(item);
    assert(hist && "ChangeHistogramPropertiesCommand: item is not a HistogramSeries");
    oldBinCount_ = hist->binCount();
    oldBinRule_ = hist->binRule();
    oldNormalization_ = hist->normalization();
    oldFillColor_ = hist->fillColor();
    oldName_ = hist->name();
    oldVisible_ = hist->isVisible();
}

void ChangeHistogramPropertiesCommand::execute() {
    auto* item = scene_->itemAt(itemIndex_);
    auto* hist = dynamic_cast<plot::HistogramSeries*>(item);
    assert(hist);
    if (newBinCount_ > 0) {
        hist->setBinCount(newBinCount_);
    } else {
        hist->setAutoBinning(newBinRule_);
    }
    hist->setNormalization(newNormalization_);
    hist->setFillColor(newFillColor_);
    hist->setName(newName_);
    hist->setVisible(newVisible_);
}

void ChangeHistogramPropertiesCommand::undo() {
    auto* item = scene_->itemAt(itemIndex_);
    auto* hist = dynamic_cast<plot::HistogramSeries*>(item);
    assert(hist);
    if (oldBinCount_ > 0) {
        hist->setBinCount(oldBinCount_);
    } else {
        hist->setAutoBinning(oldBinRule_);
    }
    hist->setNormalization(oldNormalization_);
    hist->setFillColor(oldFillColor_);
    hist->setName(oldName_);
    hist->setVisible(oldVisible_);
}

QString ChangeHistogramPropertiesCommand::description() const {
    return QStringLiteral("Change histogram properties of \"%1\"")
        .arg(newName_.isEmpty()
                 ? QStringLiteral("Item %1").arg(itemIndex_)
                 : newName_);
}

}  // namespace lumen::core::commands
