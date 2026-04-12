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
/// single ViolinSeries within a PlotScene.  Captures old values on
/// construction so that undo() can restore them.
class ChangeViolinPropertiesCommand : public Command {
public:
    ChangeViolinPropertiesCommand(plot::PlotScene* scene,
                                  std::size_t itemIndex, double newBandwidth,
                                  bool newAutoKde, bool newSplit,
                                  QColor newFillColor, QString newName,
                                  bool newVisible);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    plot::PlotScene* scene_;
    std::size_t itemIndex_;
    double oldBandwidth_;
    double newBandwidth_;
    bool oldAutoKde_;
    bool newAutoKde_;
    bool oldSplit_;
    bool newSplit_;
    QColor oldFillColor_;
    QColor newFillColor_;
    QString oldName_;
    QString newName_;
    bool oldVisible_;
    bool newVisible_;
};

}  // namespace lumen::core::commands
