#pragma once

#include <core/reactive/ReactiveMode.h>
#include <plot/PlotStyle.h>

#include <QDockWidget>
#include <QHash>
#include <QString>

#include <memory>
#include <vector>

class QComboBox;
class QToolBar;
class QPushButton;
class QHBoxLayout;

namespace lumen::core {
class CommandBus;
class PlotRegistry;
}

namespace lumen::data {
class TabularBundle;
}

namespace lumen::plot {
class PlotScene;
}

namespace lumen::reactive {
class ReactiveBinding;
}

namespace lumen::ui {

class PlotCanvas;
class ReactivityModeWidget;

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

    /// Set the PlotRegistry for document-to-canvas tracking.
    void setPlotRegistry(core::PlotRegistry* registry);

    /// Set the CommandBus for undo-able property changes.
    void setCommandBus(core::CommandBus* bus);

    /// Set the TabularBundle to plot. Populates column pickers and creates
    /// default plot (first numeric col as X, second as Y).
    /// @p documentPath is used to register the canvas in PlotRegistry.
    void setDataFrame(const data::TabularBundle* bundle, const QString& documentPath = {});

    /// Access the canvas widget.
    [[nodiscard]] PlotCanvas* canvas() const { return canvas_; }

    /// Access the underlying PlotScene (for workspace serialization).
    [[nodiscard]] plot::PlotScene* scene() const { return scene_.get(); }

    /// Set the ReactiveBinding used by this dock's plot.
    void setReactiveBinding(reactive::ReactiveBinding* binding);

    /// Access the reactivity mode widget.
    [[nodiscard]] ReactivityModeWidget* reactivityWidget() const { return reactivityWidget_; }

private slots:
    void onReactivityModeChanged(reactive::Mode m);
    void onSeriesDoubleClicked(int seriesIndex);
    void onEmptyAreaDoubleClicked();
    void onXAxisDoubleClicked();
    void onYAxisDoubleClicked();
    void onTitleDoubleClicked();
    void onLegendDoubleClicked();
    void onTitleEditFinished(const QString& newTitle);

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
        QComboBox* typeCombo = nullptr;  // per-entry Line/Scatter/Bar
        QPushButton* removeBtn = nullptr;
    };
    std::vector<YSeriesEntry> yEntries_;
    QPushButton* addSeriesBtn_ = nullptr;
    QWidget* yContainer_ = nullptr;
    QHBoxLayout* yLayout_ = nullptr;
    QComboBox* plotTypeCombo_ = nullptr;

    core::CommandBus* commandBus_ = nullptr;
    core::PlotRegistry* registry_ = nullptr;
    const data::TabularBundle* bundle_ = nullptr;
    QString documentPath_;
    std::unique_ptr<plot::PlotScene> scene_;
    QStringList numericColumns_;

    // Style persistence across column changes (T5).
    QHash<QString, plot::PlotStyle> customStyles_;
    QHash<QString, bool> customVisibility_;
    QHash<QString, QString> customNames_;

    // Reactivity mode (Phase 7).
    ReactivityModeWidget* reactivityWidget_ = nullptr;
    reactive::ReactiveBinding* reactiveBinding_ = nullptr;
};

}  // namespace lumen::ui
