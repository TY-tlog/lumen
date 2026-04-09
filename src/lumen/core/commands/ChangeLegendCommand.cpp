#include "ChangeLegendCommand.h"

#include "plot/Legend.h"

namespace lumen::core::commands {

ChangeLegendCommand::ChangeLegendCommand(plot::Legend* legend,
                                         plot::LegendPosition newPosition,
                                         bool newVisible)
    : legend_(legend)
    , newPos_(newPosition)
    , newVisible_(newVisible) {
    // Capture old values from the current state of the legend.
    oldPos_ = legend_->position();
    oldVisible_ = legend_->isVisible();
}

void ChangeLegendCommand::execute() {
    legend_->setPosition(newPos_);
    legend_->setVisible(newVisible_);
}

void ChangeLegendCommand::undo() {
    legend_->setPosition(oldPos_);
    legend_->setVisible(oldVisible_);
}

QString ChangeLegendCommand::description() const {
    return QStringLiteral("Change legend properties");
}

}  // namespace lumen::core::commands
