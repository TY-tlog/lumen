#pragma once

#include "core/Command.h"

#include <QString>

namespace lumen::plot {
class Axis;
enum class RangeMode;
enum class TickFormat;
}  // namespace lumen::plot

namespace lumen::core::commands {

/// Command that changes all editable properties of a single Axis.
/// Captures old values on construction so that undo() can restore them.
class ChangeAxisPropertiesCommand : public Command {
public:
    ChangeAxisPropertiesCommand(plot::Axis* axis, QString newLabel,
                                plot::RangeMode newRangeMode,
                                double newManualMin, double newManualMax,
                                int newTickCount, plot::TickFormat newTickFormat,
                                int newTickFormatDecimals, bool newGridVisible);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    plot::Axis* axis_;

    // Old values captured from axis_ in constructor.
    QString oldLabel_;
    plot::RangeMode oldRangeMode_;
    double oldManualMin_;
    double oldManualMax_;
    int oldTickCount_;
    plot::TickFormat oldTickFormat_;
    int oldTickFormatDecimals_;
    bool oldGridVisible_;

    // New values stored from constructor params.
    QString newLabel_;
    plot::RangeMode newRangeMode_;
    double newManualMin_;
    double newManualMax_;
    int newTickCount_;
    plot::TickFormat newTickFormat_;
    int newTickFormatDecimals_;
    bool newGridVisible_;
};

}  // namespace lumen::core::commands
