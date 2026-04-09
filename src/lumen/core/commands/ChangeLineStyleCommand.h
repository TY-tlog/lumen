#pragma once

#include "core/Command.h"
#include "plot/PlotStyle.h"

#include <QString>

#include <cstddef>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core::commands {

/// Command that changes the visual style, name, and visibility of a
/// single LineSeries within a PlotScene.  Captures old values on
/// construction so that undo() can restore them.
class ChangeLineStyleCommand : public Command {
public:
    ChangeLineStyleCommand(plot::PlotScene* scene, std::size_t seriesIndex,
                           plot::PlotStyle newStyle, QString newName,
                           bool newVisible);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    plot::PlotScene* scene_;
    std::size_t seriesIndex_;
    plot::PlotStyle oldStyle_;
    plot::PlotStyle newStyle_;
    QString oldName_;
    QString newName_;
    bool oldVisible_;
    bool newVisible_;
};

}  // namespace lumen::core::commands
