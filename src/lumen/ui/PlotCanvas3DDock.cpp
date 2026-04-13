#include "PlotCanvas3DDock.h"

#include "PlotCanvas3D.h"

namespace lumen::ui {

PlotCanvas3DDock::PlotCanvas3DDock(QWidget* parent)
    : QDockWidget(tr("3D View"), parent) {
    setObjectName(QStringLiteral("PlotCanvas3DDock"));

    canvas_ = new PlotCanvas3D(this);
    setWidget(canvas_);
}

}  // namespace lumen::ui
