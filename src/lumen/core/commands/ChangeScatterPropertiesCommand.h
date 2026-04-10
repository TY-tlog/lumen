#pragma once

#include "core/Command.h"
#include "plot/ScatterSeries.h"

#include <QColor>
#include <QString>

#include <cstddef>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core::commands {

/// Command that changes the visual properties, name, and visibility of a
/// single ScatterSeries within a PlotScene.  Captures old values on
/// construction so that undo() can restore them.
class ChangeScatterPropertiesCommand : public Command {
public:
    ChangeScatterPropertiesCommand(plot::PlotScene* scene,
                                   std::size_t seriesIndex, QColor newColor,
                                   plot::MarkerShape newMarkerShape,
                                   int newMarkerSize, bool newFilled,
                                   QString newName, bool newVisible);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    plot::PlotScene* scene_;
    std::size_t seriesIndex_;
    QColor oldColor_;
    QColor newColor_;
    plot::MarkerShape oldMarkerShape_;
    plot::MarkerShape newMarkerShape_;
    int oldMarkerSize_;
    int newMarkerSize_;
    bool oldFilled_;
    bool newFilled_;
    QString oldName_;
    QString newName_;
    bool oldVisible_;
    bool newVisible_;
};

}  // namespace lumen::core::commands
