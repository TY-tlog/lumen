#pragma once

#include "GridLayout.h"
#include "PanelConfig.h"

#include <style/types.h>

#include <QObject>

#include <memory>
#include <vector>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::dashboard {

class Dashboard : public QObject {
    Q_OBJECT

public:
    explicit Dashboard(QObject* parent = nullptr);
    ~Dashboard() override;

    int addPanel(PanelConfig config);
    void removePanel(int index);
    [[nodiscard]] int panelCount() const;
    [[nodiscard]] const PanelConfig& panelConfigAt(int index) const;
    [[nodiscard]] PanelConfig& panelConfigAt(int index);
    [[nodiscard]] plot::PlotScene* sceneAt(int index);

    void setGridSize(int rows, int cols);
    [[nodiscard]] int gridRows() const;
    [[nodiscard]] int gridCols() const;
    [[nodiscard]] const GridLayout& layout() const { return layout_; }

    void setDashboardStyle(const style::Style& style);
    [[nodiscard]] const style::Style& dashboardStyle() const { return dashboardStyle_; }

    void setPanelStyle(int index, const style::Style& style);
    [[nodiscard]] const style::Style& panelStyle(int index) const;

signals:
    void panelAdded(int index);
    void panelRemoved(int index);
    void layoutChanged();
    void styleChanged();

private:
    struct Panel {
        PanelConfig config;
        std::unique_ptr<plot::PlotScene> scene;
        style::Style panelStyle;
    };

    std::vector<Panel> panels_;
    GridLayout layout_;
    style::Style dashboardStyle_;
};

}  // namespace lumen::dashboard
