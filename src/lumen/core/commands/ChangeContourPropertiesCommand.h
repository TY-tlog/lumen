#pragma once

#include "core/Command.h"

#include <QColor>
#include <QString>

#include <cstddef>
#include <vector>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core::commands {

/// Command that changes the visual properties, name, and visibility of a
/// single ContourPlot within a PlotScene.  Captures old values on
/// construction so that undo() can restore them.
class ChangeContourPropertiesCommand : public Command {
public:
    ChangeContourPropertiesCommand(plot::PlotScene* scene,
                                   std::size_t itemIndex,
                                   std::vector<double> newLevels,
                                   int newAutoLevelCount, QColor newLineColor,
                                   double newLineWidth, bool newLabelsVisible,
                                   QString newName, bool newVisible);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    plot::PlotScene* scene_;
    std::size_t itemIndex_;
    std::vector<double> oldLevels_;
    std::vector<double> newLevels_;
    int oldAutoLevelCount_;
    int newAutoLevelCount_;
    QColor oldLineColor_;
    QColor newLineColor_;
    double oldLineWidth_;
    double newLineWidth_;
    bool oldLabelsVisible_;
    bool newLabelsVisible_;
    QString oldName_;
    QString newName_;
    bool oldVisible_;
    bool newVisible_;
};

}  // namespace lumen::core::commands
