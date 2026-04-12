#pragma once

#include "core/Command.h"
#include "plot/Colormap.h"
#include "plot/Heatmap.h"

#include <QString>

#include <cstddef>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core::commands {

/// Command that changes the visual properties, name, and visibility of a
/// single Heatmap within a PlotScene.  Captures old values on
/// construction so that undo() can restore them.
class ChangeHeatmapPropertiesCommand : public Command {
public:
    ChangeHeatmapPropertiesCommand(plot::PlotScene* scene,
                                   std::size_t itemIndex,
                                   plot::Colormap newColormap, double newValueMin,
                                   double newValueMax, bool newAutoRange,
                                   plot::Heatmap::Interpolation newInterpolation,
                                   double newOpacity, QString newName,
                                   bool newVisible);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    plot::PlotScene* scene_;
    std::size_t itemIndex_;
    plot::Colormap oldColormap_;
    plot::Colormap newColormap_;
    double oldValueMin_;
    double newValueMin_;
    double oldValueMax_;
    double newValueMax_;
    bool oldAutoRange_;
    bool newAutoRange_;
    plot::Heatmap::Interpolation oldInterpolation_;
    plot::Heatmap::Interpolation newInterpolation_;
    double oldOpacity_;
    double newOpacity_;
    QString oldName_;
    QString newName_;
    bool oldVisible_;
    bool newVisible_;
};

}  // namespace lumen::core::commands
