#pragma once

#include "core/Command.h"
#include "plot/HistogramSeries.h"

#include <QColor>
#include <QString>

#include <cstddef>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core::commands {

/// Command that changes the visual properties, name, and visibility of a
/// single HistogramSeries within a PlotScene.  Captures old values on
/// construction so that undo() can restore them.
class ChangeHistogramPropertiesCommand : public Command {
public:
    ChangeHistogramPropertiesCommand(plot::PlotScene* scene,
                                     std::size_t itemIndex, int newBinCount,
                                     plot::HistogramSeries::BinRule newBinRule,
                                     plot::HistogramSeries::Normalization newNormalization,
                                     QColor newFillColor, QString newName,
                                     bool newVisible);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    plot::PlotScene* scene_;
    std::size_t itemIndex_;
    int oldBinCount_;
    int newBinCount_;
    plot::HistogramSeries::BinRule oldBinRule_;
    plot::HistogramSeries::BinRule newBinRule_;
    plot::HistogramSeries::Normalization oldNormalization_;
    plot::HistogramSeries::Normalization newNormalization_;
    QColor oldFillColor_;
    QColor newFillColor_;
    QString oldName_;
    QString newName_;
    bool oldVisible_;
    bool newVisible_;
};

}  // namespace lumen::core::commands
