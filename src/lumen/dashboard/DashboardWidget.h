#pragma once

#include <QWidget>

#include <vector>

namespace lumen::ui {
class PlotCanvas;
}

namespace lumen::dashboard {

class Dashboard;

class DashboardWidget : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWidget(Dashboard* dashboard,
                              QWidget* parent = nullptr);

    [[nodiscard]] Dashboard* dashboard() const { return dashboard_; }

    [[nodiscard]] ui::PlotCanvas* canvasAt(int index) const;

public slots:
    void layoutPanels();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onPanelAdded(int index);
    void onPanelRemoved(int index);

private:
    Dashboard* dashboard_;
    std::vector<ui::PlotCanvas*> canvases_;
};

}  // namespace lumen::dashboard
