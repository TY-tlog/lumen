#pragma once

#include "core/Command.h"

namespace lumen::plot {
class Legend;
enum class LegendPosition;
}  // namespace lumen::plot

namespace lumen::core::commands {

/// Command that changes legend position and visibility.
/// Captures old values on construction so that undo() can restore them.
class ChangeLegendCommand : public Command {
public:
    ChangeLegendCommand(plot::Legend* legend, plot::LegendPosition newPosition,
                        bool newVisible);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    plot::Legend* legend_;
    plot::LegendPosition oldPos_;
    plot::LegendPosition newPos_;
    bool oldVisible_;
    bool newVisible_;
};

}  // namespace lumen::core::commands
