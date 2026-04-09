#include "PlotCanvasDock.h"

#include "AxisDialog.h"
#include "InteractionController.h"
#include "LegendDialog.h"
#include "LinePropertyDialog.h"
#include "PlotCanvas.h"
#include "TitleDialog.h"

#include <core/CommandBus.h>
#include <core/PlotRegistry.h>
#include <core/commands/ChangeAxisPropertiesCommand.h>
#include <core/commands/ChangeLegendCommand.h>
#include <core/commands/ChangeLineStyleCommand.h>
#include <core/commands/ChangeTitleCommand.h>
#include <data/Column.h>
#include <data/ColumnType.h>
#include <data/DataFrame.h>
#include <plot/LineSeries.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>

#include <memory>

namespace lumen::ui {

PlotCanvasDock::PlotCanvasDock(QWidget* parent)
    : QDockWidget(tr("Plot"), parent)
    , scene_(std::make_unique<plot::PlotScene>()) {
    setObjectName("PlotCanvasDock");

    auto* container = new QWidget(this);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    buildToolBar();
    layout->addWidget(toolBar_);

    canvas_ = new PlotCanvas(container);
    canvas_->setPlotScene(scene_.get());
    layout->addWidget(canvas_, 1);

    // Connect double-click signals from InteractionController.
    connect(canvas_->controller(), &InteractionController::seriesDoubleClicked,
            this, &PlotCanvasDock::onSeriesDoubleClicked);
    connect(canvas_->controller(), &InteractionController::emptyAreaDoubleClicked,
            this, &PlotCanvasDock::onEmptyAreaDoubleClicked);
    connect(canvas_->controller(), &InteractionController::xAxisDoubleClicked,
            this, &PlotCanvasDock::onXAxisDoubleClicked);
    connect(canvas_->controller(), &InteractionController::yAxisDoubleClicked,
            this, &PlotCanvasDock::onYAxisDoubleClicked);
    connect(canvas_->controller(), &InteractionController::titleDoubleClicked,
            this, &PlotCanvasDock::onTitleDoubleClicked);
    connect(canvas_->controller(), &InteractionController::legendDoubleClicked,
            this, &PlotCanvasDock::onLegendDoubleClicked);

    // Connect inline title editor result.
    connect(canvas_, &PlotCanvas::titleEditFinished,
            this, &PlotCanvasDock::onTitleEditFinished);

    setWidget(container);
}

PlotCanvasDock::~PlotCanvasDock() = default;

void PlotCanvasDock::buildToolBar() {
    toolBar_ = new QToolBar(this);
    toolBar_->setMovable(false);
    toolBar_->setIconSize(QSize(16, 16));

    // X column picker.
    toolBar_->addWidget(new QLabel(QStringLiteral(" X: "), toolBar_));
    xCombo_ = new QComboBox(toolBar_);
    xCombo_->setMinimumWidth(120);
    toolBar_->addWidget(xCombo_);
    connect(xCombo_, &QComboBox::currentIndexChanged, this, [this]() {
        rebuildPlot();
    });

    toolBar_->addSeparator();

    // Y column area.
    toolBar_->addWidget(new QLabel(QStringLiteral(" Y: "), toolBar_));

    yContainer_ = new QWidget(toolBar_);
    yLayout_ = new QHBoxLayout(yContainer_);
    yLayout_->setContentsMargins(0, 0, 0, 0);
    yLayout_->setSpacing(4);
    toolBar_->addWidget(yContainer_);

    toolBar_->addSeparator();

    // Add Series button.
    addSeriesBtn_ = new QPushButton(QStringLiteral("+Series"), toolBar_);
    addSeriesBtn_->setToolTip(tr("Add another Y series"));
    toolBar_->addWidget(addSeriesBtn_);
    connect(addSeriesBtn_, &QPushButton::clicked, this, [this]() {
        addYSeries();
    });
}

void PlotCanvasDock::setPlotRegistry(core::PlotRegistry* registry) {
    registry_ = registry;
}

void PlotCanvasDock::setCommandBus(core::CommandBus* bus) {
    commandBus_ = bus;
}

void PlotCanvasDock::onSeriesDoubleClicked(int seriesIndex) {
    if (!scene_ || seriesIndex < 0 ||
        seriesIndex >= static_cast<int>(scene_->seriesCount())) {
        return;
    }

    auto& series = scene_->seriesAt(static_cast<std::size_t>(seriesIndex));
    LinePropertyDialog dialog(this);
    dialog.setStyle(series.style(), series.name(), series.isVisible());

    if (dialog.exec() == QDialog::Accepted) {
        // Persist custom style for this column name (T5).
        QString colName = series.name();
        customStyles_[colName] = dialog.resultStyle();
        customVisibility_[colName] = dialog.resultVisible();
        customNames_[colName] = dialog.resultName();

        if (commandBus_ != nullptr) {
            auto cmd = std::make_unique<core::commands::ChangeLineStyleCommand>(
                scene_.get(), static_cast<std::size_t>(seriesIndex),
                dialog.resultStyle(), dialog.resultName(),
                dialog.resultVisible());
            commandBus_->execute(std::move(cmd));
        } else {
            // Fallback without undo.
            series.setStyle(dialog.resultStyle());
            series.setName(dialog.resultName());
            series.setVisible(dialog.resultVisible());
        }
        canvas_->update();
    }
}

void PlotCanvasDock::onEmptyAreaDoubleClicked() {
    if (scene_ != nullptr && scene_->seriesCount() > 0) {
        scene_->autoRange();
        canvas_->update();
    }
}

void PlotCanvasDock::onXAxisDoubleClicked() {
    if (!scene_) {
        return;
    }
    auto& axis = scene_->xAxis();
    AxisDialog dialog(this);
    dialog.setAxisProperties(axis.label(), axis.rangeMode(),
                             axis.manualMin(), axis.manualMax(),
                             axis.tickCount(), axis.tickFormat(),
                             axis.tickFormatDecimals(), axis.gridVisible());
    if (dialog.exec() == QDialog::Accepted) {
        if (commandBus_ != nullptr) {
            auto cmd = std::make_unique<core::commands::ChangeAxisPropertiesCommand>(
                &axis, dialog.resultLabel(), dialog.resultRangeMode(),
                dialog.resultManualMin(), dialog.resultManualMax(),
                dialog.resultTickCount(), dialog.resultTickFormat(),
                dialog.resultTickFormatDecimals(), dialog.resultGridVisible());
            commandBus_->execute(std::move(cmd));
        } else {
            axis.setLabel(dialog.resultLabel());
            axis.setRangeMode(dialog.resultRangeMode());
            axis.setManualRange(dialog.resultManualMin(), dialog.resultManualMax());
            axis.setTickCount(dialog.resultTickCount());
            axis.setTickFormat(dialog.resultTickFormat());
            axis.setTickFormatDecimals(dialog.resultTickFormatDecimals());
            axis.setGridVisible(dialog.resultGridVisible());
        }
        canvas_->update();
    }
}

void PlotCanvasDock::onYAxisDoubleClicked() {
    if (!scene_) {
        return;
    }
    auto& axis = scene_->yAxis();
    AxisDialog dialog(this);
    dialog.setAxisProperties(axis.label(), axis.rangeMode(),
                             axis.manualMin(), axis.manualMax(),
                             axis.tickCount(), axis.tickFormat(),
                             axis.tickFormatDecimals(), axis.gridVisible());
    if (dialog.exec() == QDialog::Accepted) {
        if (commandBus_ != nullptr) {
            auto cmd = std::make_unique<core::commands::ChangeAxisPropertiesCommand>(
                &axis, dialog.resultLabel(), dialog.resultRangeMode(),
                dialog.resultManualMin(), dialog.resultManualMax(),
                dialog.resultTickCount(), dialog.resultTickFormat(),
                dialog.resultTickFormatDecimals(), dialog.resultGridVisible());
            commandBus_->execute(std::move(cmd));
        } else {
            axis.setLabel(dialog.resultLabel());
            axis.setRangeMode(dialog.resultRangeMode());
            axis.setManualRange(dialog.resultManualMin(), dialog.resultManualMax());
            axis.setTickCount(dialog.resultTickCount());
            axis.setTickFormat(dialog.resultTickFormat());
            axis.setTickFormatDecimals(dialog.resultTickFormatDecimals());
            axis.setGridVisible(dialog.resultGridVisible());
        }
        canvas_->update();
    }
}

void PlotCanvasDock::onTitleDoubleClicked() {
    if (!scene_) {
        return;
    }
    canvas_->startTitleEdit();
}

void PlotCanvasDock::onTitleEditFinished(const QString& newTitle) {
    if (!scene_) {
        return;
    }
    if (commandBus_ != nullptr) {
        auto cmd = std::make_unique<core::commands::ChangeTitleCommand>(
            scene_.get(), newTitle, scene_->titleFontPx(), scene_->titleWeight());
        commandBus_->execute(std::move(cmd));
    } else {
        scene_->setTitle(newTitle);
    }
    canvas_->update();
}

void PlotCanvasDock::onLegendDoubleClicked() {
    if (!scene_) {
        return;
    }
    auto& legend = scene_->legend();

    // Collect series names and colors.
    QStringList seriesNames;
    QList<QColor> seriesColors;
    for (std::size_t i = 0; i < scene_->seriesCount(); ++i) {
        const auto& s = scene_->series()[i];
        seriesNames.append(s.name());
        seriesColors.append(s.style().color);
    }

    LegendDialog dialog(this);
    dialog.setLegendProperties(legend.position(), legend.isVisible(),
                               seriesNames, seriesColors);
    if (dialog.exec() == QDialog::Accepted) {
        if (commandBus_ != nullptr) {
            auto cmd = std::make_unique<core::commands::ChangeLegendCommand>(
                &legend, dialog.resultPosition(), dialog.resultVisible());
            commandBus_->execute(std::move(cmd));
        } else {
            legend.setPosition(dialog.resultPosition());
            legend.setVisible(dialog.resultVisible());
        }

        // Apply any renamed series.
        QStringList newNames = dialog.resultSeriesNames();
        for (int i = 0; i < newNames.size() &&
             i < static_cast<int>(scene_->seriesCount()); ++i) {
            if (newNames.at(i) != seriesNames.at(i)) {
                scene_->seriesAt(static_cast<std::size_t>(i)).setName(newNames.at(i));
            }
        }

        canvas_->update();
    }
}

void PlotCanvasDock::setDataFrame(const data::DataFrame* df, const QString& documentPath) {
    documentPath_ = documentPath;
    dataFrame_ = df;

    // Find numeric columns.
    numericColumns_.clear();
    if (df != nullptr) {
        for (std::size_t i = 0; i < df->columnCount(); ++i) {
            const auto& col = df->column(i);
            if (col.type() == data::ColumnType::Double ||
                col.type() == data::ColumnType::Int64) {
                numericColumns_.append(col.name());
            }
        }
    }

    // Block signals while populating.
    xCombo_->blockSignals(true);
    xCombo_->clear();
    xCombo_->addItems(numericColumns_);
    xCombo_->blockSignals(false);

    // Clear existing Y entries.
    for (auto& entry : yEntries_) {
        delete entry.combo;
        delete entry.removeBtn;
    }
    yEntries_.clear();

    // Create default Y entry.
    if (numericColumns_.size() >= 2) {
        addYSeries();
        // Select second column for first Y.
        if (!yEntries_.empty() && yEntries_[0].combo->count() > 1) {
            yEntries_[0].combo->setCurrentIndex(1);
        }
    } else if (!numericColumns_.isEmpty()) {
        addYSeries();
    }

    rebuildPlot();

    // Register canvas in PlotRegistry if available.
    if (registry_ != nullptr && !documentPath_.isEmpty()) {
        registry_->registerPlot(documentPath_, canvas_);
    }
}

void PlotCanvasDock::addYSeries() {
    auto* combo = new QComboBox(yContainer_);
    combo->setMinimumWidth(120);
    combo->addItems(numericColumns_);

    // Default to next unused column.
    int defaultIdx = static_cast<int>(yEntries_.size()) + 1;
    if (defaultIdx < combo->count()) {
        combo->setCurrentIndex(defaultIdx);
    }

    connect(combo, &QComboBox::currentIndexChanged, this, [this]() {
        rebuildPlot();
    });

    QPushButton* removeBtn = nullptr;
    // Only show remove button for non-first entries.
    if (!yEntries_.empty()) {
        removeBtn = new QPushButton(QStringLiteral("×"), yContainer_);
        removeBtn->setFixedWidth(24);
        removeBtn->setToolTip(tr("Remove this series"));
        int idx = static_cast<int>(yEntries_.size());
        connect(removeBtn, &QPushButton::clicked, this, [this, idx]() {
            removeYSeries(idx);
        });
    }

    yLayout_->addWidget(combo);
    if (removeBtn != nullptr) {
        yLayout_->addWidget(removeBtn);
    }

    yEntries_.push_back({combo, removeBtn});
}

void PlotCanvasDock::removeYSeries(int index) {
    if (index < 0 || index >= static_cast<int>(yEntries_.size())) {
        return;
    }

    auto& entry = yEntries_[static_cast<std::size_t>(index)];
    delete entry.combo;
    delete entry.removeBtn;
    yEntries_.erase(yEntries_.begin() + index);

    rebuildPlot();
}

void PlotCanvasDock::rebuildPlot() {
    scene_->clearSeries();

    if (dataFrame_ == nullptr || numericColumns_.isEmpty()) {
        canvas_->update();
        return;
    }

    QString xName = xCombo_->currentText();
    const auto* xCol = dataFrame_->columnByName(xName);
    if (xCol == nullptr) {
        canvas_->update();
        return;
    }

    int seriesIdx = 0;
    QStringList yNames;
    for (const auto& entry : yEntries_) {
        if (entry.combo == nullptr) {
            continue;
        }
        QString yName = entry.combo->currentText();
        if (yName.isEmpty() || yName == xName) {
            continue;
        }
        const auto* yCol = dataFrame_->columnByName(yName);
        if (yCol == nullptr) {
            continue;
        }

        // Both columns must be Double for LineSeries.
        if (xCol->type() != data::ColumnType::Double ||
            yCol->type() != data::ColumnType::Double) {
            continue;
        }

        scene_->addSeries(plot::LineSeries(
            xCol, yCol, plot::PlotStyle::fromPalette(seriesIdx), yName));

        // Apply persisted custom style if available (T5).
        auto& addedSeries = scene_->seriesAt(scene_->seriesCount() - 1);
        if (customStyles_.contains(yName)) {
            addedSeries.setStyle(customStyles_[yName]);
        }
        if (customVisibility_.contains(yName)) {
            addedSeries.setVisible(customVisibility_[yName]);
        }
        if (customNames_.contains(yName)) {
            addedSeries.setName(customNames_[yName]);
        }

        yNames.append(yName);
        ++seriesIdx;
    }

    if (scene_->seriesCount() > 0) {
        scene_->autoRange();
        // Title: "Y vs X" or "Y1, Y2 vs X".
        scene_->setTitle(yNames.join(QStringLiteral(", ")) +
                         QStringLiteral(" vs ") + xName);
        scene_->xAxis().setLabel(xName);
        if (yNames.size() == 1) {
            scene_->yAxis().setLabel(yNames.first());
        } else {
            scene_->yAxis().setLabel(QString());
        }
    }

    canvas_->update();
}

}  // namespace lumen::ui
