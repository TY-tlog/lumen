#pragma once

#include "core/Command.h"

#include <QColor>
#include <QString>

#include <cstddef>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core::commands {

/// Command that changes the visual properties, name, and visibility of a
/// single BarSeries within a PlotScene.  Captures old values on
/// construction so that undo() can restore them.
class ChangeBarPropertiesCommand : public Command {
public:
    ChangeBarPropertiesCommand(plot::PlotScene* scene,
                               std::size_t seriesIndex, QColor newFillColor,
                               QColor newOutlineColor, double newBarWidth,
                               QString newName, bool newVisible);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    plot::PlotScene* scene_;
    std::size_t seriesIndex_;
    QColor oldFillColor_;
    QColor newFillColor_;
    QColor oldOutlineColor_;
    QColor newOutlineColor_;
    double oldBarWidth_;
    double newBarWidth_;
    QString oldName_;
    QString newName_;
    bool oldVisible_;
    bool newVisible_;
};

}  // namespace lumen::core::commands
