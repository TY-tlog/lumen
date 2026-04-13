#pragma once

#include "plot/Annotation.h"

#include <QDialog>

class QDialogButtonBox;
class QStackedWidget;

namespace lumen::core {
class CommandBus;
}

namespace lumen::plot {
class PlotScene;
}

namespace lumen::ui {

/// Property dialog for annotations — shows type-specific controls.
class AnnotationPropertyDialog : public QDialog {
    Q_OBJECT

public:
    explicit AnnotationPropertyDialog(plot::Annotation* annotation,
                                       plot::PlotScene* scene,
                                       core::CommandBus* bus,
                                       QWidget* parent = nullptr);

private:
    void onAccepted();

    plot::Annotation* annotation_ = nullptr;
    plot::PlotScene* scene_ = nullptr;
    core::CommandBus* bus_ = nullptr;
    QStackedWidget* stack_ = nullptr;
    QDialogButtonBox* buttonBox_ = nullptr;
};

}  // namespace lumen::ui
