#include "DashboardWidget.h"
#include "Dashboard.h"

#include <ui/PlotCanvas.h>

#include <QResizeEvent>

namespace lumen::dashboard {

DashboardWidget::DashboardWidget(Dashboard* dashboard, QWidget* parent)
    : QWidget(parent)
    , dashboard_(dashboard)
{
    connect(dashboard_, &Dashboard::panelAdded,
            this, &DashboardWidget::onPanelAdded);
    connect(dashboard_, &Dashboard::panelRemoved,
            this, &DashboardWidget::onPanelRemoved);
    connect(dashboard_, &Dashboard::layoutChanged,
            this, &DashboardWidget::layoutPanels);

    for (int i = 0; i < dashboard_->panelCount(); ++i) {
        onPanelAdded(i);
    }
}

ui::PlotCanvas* DashboardWidget::canvasAt(int index) const
{
    if (index < 0 || index >= static_cast<int>(canvases_.size()))
        return nullptr;
    return canvases_[static_cast<std::size_t>(index)];
}

void DashboardWidget::layoutPanels()
{
    if (!dashboard_)
        return;

    const auto& layout = dashboard_->layout();
    QSizeF available(width(), height());

    for (int i = 0; i < static_cast<int>(canvases_.size()); ++i) {
        if (i >= dashboard_->panelCount())
            break;
        QRectF r = layout.cellRect(dashboard_->panelConfigAt(i), available);
        canvases_[static_cast<std::size_t>(i)]->setGeometry(r.toRect());
    }
}

void DashboardWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    layoutPanels();
}

void DashboardWidget::onPanelAdded(int index)
{
    auto* canvas = new ui::PlotCanvas(this);
    canvas->setPlotScene(dashboard_->sceneAt(index));
    canvas->show();
    canvases_.insert(canvases_.begin() + index, canvas);
    layoutPanels();
}

void DashboardWidget::onPanelRemoved(int index)
{
    if (index < 0 || index >= static_cast<int>(canvases_.size()))
        return;

    auto* canvas = canvases_[static_cast<std::size_t>(index)];
    canvases_.erase(canvases_.begin() + index);
    canvas->deleteLater();

    for (int i = index; i < static_cast<int>(canvases_.size()); ++i) {
        canvases_[static_cast<std::size_t>(i)]->setPlotScene(
            dashboard_->sceneAt(i));
    }

    layoutPanels();
}

}  // namespace lumen::dashboard
