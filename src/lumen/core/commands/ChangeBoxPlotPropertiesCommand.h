#pragma once

#include "core/Command.h"
#include "plot/BoxPlotSeries.h"

#include <QColor>
#include <QString>

#include <cstddef>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core::commands {

/// Command that changes the visual properties, name, and visibility of a
/// single BoxPlotSeries within a PlotScene.  Captures old values on
/// construction so that undo() can restore them.
class ChangeBoxPlotPropertiesCommand : public Command {
public:
    ChangeBoxPlotPropertiesCommand(plot::PlotScene* scene,
                                   std::size_t itemIndex,
                                   plot::BoxPlotSeries::WhiskerRule newWhiskerRule,
                                   bool newNotched, bool newOutliersVisible,
                                   QColor newFillColor, QString newName,
                                   bool newVisible);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    plot::PlotScene* scene_;
    std::size_t itemIndex_;
    plot::BoxPlotSeries::WhiskerRule oldWhiskerRule_;
    plot::BoxPlotSeries::WhiskerRule newWhiskerRule_;
    bool oldNotched_;
    bool newNotched_;
    bool oldOutliersVisible_;
    bool newOutliersVisible_;
    QColor oldFillColor_;
    QColor newFillColor_;
    QString oldName_;
    QString newName_;
    bool oldVisible_;
    bool newVisible_;
};

}  // namespace lumen::core::commands
