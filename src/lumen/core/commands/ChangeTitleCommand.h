#pragma once

#include "core/Command.h"

#include <QFont>
#include <QString>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core::commands {

/// Command that changes the plot title text, font size, and weight.
/// Captures old values on construction so that undo() can restore them.
class ChangeTitleCommand : public Command {
public:
    ChangeTitleCommand(plot::PlotScene* scene, QString newTitle, int newFontPx,
                       QFont::Weight newWeight);

    void execute() override;
    void undo() override;
    [[nodiscard]] QString description() const override;

private:
    plot::PlotScene* scene_;
    QString oldTitle_;
    QString newTitle_;
    int oldFontPx_;
    int newFontPx_;
    QFont::Weight oldWeight_;
    QFont::Weight newWeight_;
};

}  // namespace lumen::core::commands
