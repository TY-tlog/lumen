#pragma once

#include <QDockWidget>

#include <memory>
#include <vector>

class QComboBox;
class QToolBar;
class QPushButton;
class QHBoxLayout;

namespace lumen::data {
class DataFrame;
}

namespace lumen::plot {
class PlotScene;
}

namespace lumen::ui {

class PlotCanvas;

/// Dock widget containing a PlotCanvas and column-picker controls.
///
/// Manages a PlotScene: when setDataFrame is called, populates combo
/// boxes with numeric column names. Changing selection rebuilds the
/// scene with new LineSeries.
class PlotCanvasDock : public QDockWidget {
    Q_OBJECT

public:
    explicit PlotCanvasDock(QWidget* parent = nullptr);
    ~PlotCanvasDock() override;

    /// Set the DataFrame to plot. Populates column pickers and creates
    /// default plot (first numeric col as X, second as Y).
    void setDataFrame(const data::DataFrame* df);

    /// Access the canvas widget.
    [[nodiscard]] PlotCanvas* canvas() const { return canvas_; }

private:
    void buildToolBar();
    void rebuildPlot();
    void addYSeries();
    void removeYSeries(int index);

    PlotCanvas* canvas_ = nullptr;
    QToolBar* toolBar_ = nullptr;
    QComboBox* xCombo_ = nullptr;

    struct YSeriesEntry {
        QComboBox* combo = nullptr;
        QPushButton* removeBtn = nullptr;
    };
    std::vector<YSeriesEntry> yEntries_;
    QPushButton* addSeriesBtn_ = nullptr;
    QWidget* yContainer_ = nullptr;
    QHBoxLayout* yLayout_ = nullptr;

    const data::DataFrame* dataFrame_ = nullptr;
    std::unique_ptr<plot::PlotScene> scene_;
    QStringList numericColumns_;
};

}  // namespace lumen::ui
