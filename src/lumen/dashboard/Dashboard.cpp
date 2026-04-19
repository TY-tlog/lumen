#include "Dashboard.h"

#include <plot/PlotScene.h>

namespace lumen::dashboard {

Dashboard::Dashboard(QObject* parent)
    : QObject(parent)
{
}

Dashboard::~Dashboard() = default;

int Dashboard::addPanel(PanelConfig config)
{
    int idx = static_cast<int>(panels_.size());

    Panel panel;
    panel.config = std::move(config);
    panel.scene = std::make_unique<plot::PlotScene>();
    panels_.push_back(std::move(panel));

    emit panelAdded(idx);
    return idx;
}

void Dashboard::removePanel(int index)
{
    if (index < 0 || index >= static_cast<int>(panels_.size()))
        return;

    panels_.erase(panels_.begin() + index);
    emit panelRemoved(index);
}

int Dashboard::panelCount() const
{
    return static_cast<int>(panels_.size());
}

const PanelConfig& Dashboard::panelConfigAt(int index) const
{
    return panels_.at(static_cast<std::size_t>(index)).config;
}

PanelConfig& Dashboard::panelConfigAt(int index)
{
    return panels_.at(static_cast<std::size_t>(index)).config;
}

plot::PlotScene* Dashboard::sceneAt(int index)
{
    if (index < 0 || index >= static_cast<int>(panels_.size()))
        return nullptr;
    return panels_[static_cast<std::size_t>(index)].scene.get();
}

void Dashboard::setGridSize(int rows, int cols)
{
    layout_.setGridSize(rows, cols);
    emit layoutChanged();
}

int Dashboard::gridRows() const
{
    return layout_.rows();
}

int Dashboard::gridCols() const
{
    return layout_.cols();
}

void Dashboard::setDashboardStyle(const style::Style& style)
{
    dashboardStyle_ = style;
    emit styleChanged();
}

void Dashboard::setPanelStyle(int index, const style::Style& style)
{
    if (index < 0 || index >= static_cast<int>(panels_.size()))
        return;
    panels_[static_cast<std::size_t>(index)].panelStyle = style;
    emit styleChanged();
}

const style::Style& Dashboard::panelStyle(int index) const
{
    static const style::Style empty;
    if (index < 0 || index >= static_cast<int>(panels_.size()))
        return empty;
    return panels_[static_cast<std::size_t>(index)].panelStyle;
}

}  // namespace lumen::dashboard
