#pragma once

#include "plot/Annotation.h"

#include <QWidget>

class QButtonGroup;
class QToolButton;

namespace lumen::ui {

/// Floating toolbar for annotation placement on PlotCanvas.
/// 6 buttons: Arrow, Box, Callout, Text, ScaleBar, ColorBar.
class AnnotationToolbar : public QWidget {
    Q_OBJECT

public:
    explicit AnnotationToolbar(QWidget* parent = nullptr);

    [[nodiscard]] int buttonCount() const;

signals:
    void toolSelected(lumen::plot::Annotation::Type type);
    void toolDeselected();

private:
    void onButtonToggled(int id, bool checked);

    QButtonGroup* group_ = nullptr;
};

}  // namespace lumen::ui
