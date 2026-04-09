#include "ChangeAxisPropertiesCommand.h"

#include "plot/Axis.h"

namespace lumen::core::commands {

ChangeAxisPropertiesCommand::ChangeAxisPropertiesCommand(
    plot::Axis* axis, QString newLabel, plot::RangeMode newRangeMode,
    double newManualMin, double newManualMax, int newTickCount,
    plot::TickFormat newTickFormat, int newTickFormatDecimals,
    bool newGridVisible)
    : axis_(axis)
    , newLabel_(std::move(newLabel))
    , newRangeMode_(newRangeMode)
    , newManualMin_(newManualMin)
    , newManualMax_(newManualMax)
    , newTickCount_(newTickCount)
    , newTickFormat_(newTickFormat)
    , newTickFormatDecimals_(newTickFormatDecimals)
    , newGridVisible_(newGridVisible) {
    // Capture old values from the current state of the axis.
    oldLabel_ = axis_->label();
    oldRangeMode_ = axis_->rangeMode();
    oldManualMin_ = axis_->manualMin();
    oldManualMax_ = axis_->manualMax();
    oldTickCount_ = axis_->tickCount();
    oldTickFormat_ = axis_->tickFormat();
    oldTickFormatDecimals_ = axis_->tickFormatDecimals();
    oldGridVisible_ = axis_->gridVisible();
}

void ChangeAxisPropertiesCommand::execute() {
    axis_->setLabel(newLabel_);
    axis_->setRangeMode(newRangeMode_);
    axis_->setManualRange(newManualMin_, newManualMax_);
    axis_->setTickCount(newTickCount_);
    axis_->setTickFormat(newTickFormat_);
    axis_->setTickFormatDecimals(newTickFormatDecimals_);
    axis_->setGridVisible(newGridVisible_);
}

void ChangeAxisPropertiesCommand::undo() {
    axis_->setLabel(oldLabel_);
    axis_->setRangeMode(oldRangeMode_);
    axis_->setManualRange(oldManualMin_, oldManualMax_);
    axis_->setTickCount(oldTickCount_);
    axis_->setTickFormat(oldTickFormat_);
    axis_->setTickFormatDecimals(oldTickFormatDecimals_);
    axis_->setGridVisible(oldGridVisible_);
}

QString ChangeAxisPropertiesCommand::description() const {
    return QStringLiteral("Change axis properties");
}

}  // namespace lumen::core::commands
