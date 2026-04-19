#include "PlotCanvasDock.h"

#include "AxisDialog.h"
#include "BarPropertyDialog.h"
#include "BoxPlotPropertyDialog.h"
#include "ContourPropertyDialog.h"
#include "HeatmapPropertyDialog.h"
#include "HistogramPropertyDialog.h"
#include "InteractionController.h"
#include "LegendDialog.h"
#include "LinePropertyDialog.h"
#include "PlotCanvas.h"
#include "ReactivityModeWidget.h"
#include "ScatterPropertyDialog.h"
#include "TitleDialog.h"
#include "ViolinPropertyDialog.h"

#include <core/CommandBus.h>
#include <core/reactive/ReactiveBinding.h>
#include <core/PlotRegistry.h>
#include <core/commands/ChangeAxisPropertiesCommand.h>
#include <core/commands/ChangeBarPropertiesCommand.h>
#include <core/commands/ChangeBoxPlotPropertiesCommand.h>
#include <core/commands/ChangeContourPropertiesCommand.h>
#include <core/commands/ChangeHeatmapPropertiesCommand.h>
#include <core/commands/ChangeHistogramPropertiesCommand.h>
#include <core/commands/ChangeLegendCommand.h>
#include <core/commands/ChangeLineStyleCommand.h>
#include <core/commands/ChangeScatterPropertiesCommand.h>
#include <core/commands/ChangeTitleCommand.h>
#include <core/commands/ChangeViolinPropertiesCommand.h>
#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>
#include <plot/BarSeries.h>
#include <plot/BoxPlotSeries.h>
#include <plot/ContourPlot.h>
#include <plot/Heatmap.h>
#include <plot/HistogramSeries.h>
#include <plot/LineSeries.h>
#include <plot/PlotItem.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>
#include <plot/ScatterSeries.h>
#include <plot/ViolinSeries.h>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
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

    // Right-click context menu for plot management (A9: discoverability).
    canvas_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(canvas_, &QWidget::customContextMenuRequested,
            this, [this](const QPoint& pos) {
        QMenu menu(canvas_);
        auto* titleAct = menu.addAction(tr("Edit Title..."));
        connect(titleAct, &QAction::triggered, this, &PlotCanvasDock::onTitleDoubleClicked);
        auto* xAxisAct = menu.addAction(tr("Edit X Axis..."));
        connect(xAxisAct, &QAction::triggered, this, &PlotCanvasDock::onXAxisDoubleClicked);
        auto* yAxisAct = menu.addAction(tr("Edit Y Axis..."));
        connect(yAxisAct, &QAction::triggered, this, &PlotCanvasDock::onYAxisDoubleClicked);
        auto* legendAct = menu.addAction(tr("Edit Legend..."));
        connect(legendAct, &QAction::triggered, this, &PlotCanvasDock::onLegendDoubleClicked);
        menu.addSeparator();
        auto* autoRangeAct = menu.addAction(tr("Auto Range"));
        connect(autoRangeAct, &QAction::triggered, this, [this]() {
            if (scene_) {
                scene_->autoRange();
                canvas_->update();
            }
        });
        menu.addSeparator();
        menu.addAction(tr("Tip: Double-click elements to edit"))->setEnabled(false);
        menu.exec(canvas_->mapToGlobal(pos));
    });

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

    // Y column area (scrollable to handle many series).
    toolBar_->addWidget(new QLabel(QStringLiteral(" Y: "), toolBar_));

    yContainer_ = new QWidget(toolBar_);
    yLayout_ = new QHBoxLayout(yContainer_);
    yLayout_->setContentsMargins(0, 0, 0, 0);
    yLayout_->setSpacing(4);

    auto* scrollArea = new QScrollArea(toolBar_);
    scrollArea->setWidget(yContainer_);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setMaximumHeight(32);
    toolBar_->addWidget(scrollArea);

    toolBar_->addSeparator();

    // Add Series button.
    addSeriesBtn_ = new QPushButton(QStringLiteral("+Series"), toolBar_);
    addSeriesBtn_->setToolTip(tr("Add another Y series"));
    toolBar_->addWidget(addSeriesBtn_);
    connect(addSeriesBtn_, &QPushButton::clicked, this, [this]() {
        addYSeries();
    });

    toolBar_->addSeparator();

    // Reactivity mode selector (Phase 7 / T3).
    reactivityWidget_ = new ReactivityModeWidget(toolBar_);
    toolBar_->addWidget(reactivityWidget_);
    connect(reactivityWidget_, &ReactivityModeWidget::modeChanged,
            this, &PlotCanvasDock::onReactivityModeChanged);
}

void PlotCanvasDock::setPlotRegistry(core::PlotRegistry* registry) {
    registry_ = registry;
}

void PlotCanvasDock::setCommandBus(core::CommandBus* bus) {
    commandBus_ = bus;
}

void PlotCanvasDock::setReactiveBinding(reactive::ReactiveBinding* binding) {
    reactiveBinding_ = binding;
    if (reactiveBinding_ != nullptr && reactivityWidget_ != nullptr) {
        reactivityWidget_->setMode(reactiveBinding_->mode());
    }
}

void PlotCanvasDock::onReactivityModeChanged(reactive::Mode m) {
    if (reactiveBinding_ != nullptr) {
        reactiveBinding_->setMode(m);
    }
}

void PlotCanvasDock::onSeriesDoubleClicked(int seriesIndex) {
    if (!scene_ || seriesIndex < 0 ||
        seriesIndex >= static_cast<int>(scene_->itemCount())) {
        return;
    }

    auto idx = static_cast<std::size_t>(seriesIndex);
    auto* item = scene_->itemAt(idx);

    // T12: Dispatch based on item type.
    switch (item->type()) {
    case plot::PlotItem::Type::Scatter: {
        auto* scatter = dynamic_cast<plot::ScatterSeries*>(item);
        if (scatter == nullptr) { return; }
        ScatterPropertyDialog dialog(this);
        dialog.setProperties(scatter->color(), scatter->markerShape(),
                             scatter->markerSize(), scatter->filled(),
                             scatter->name(), scatter->isVisible());
        if (dialog.exec() == QDialog::Accepted) {
            customVisibility_[scatter->name()] = dialog.resultVisible();
            customNames_[scatter->name()] = dialog.resultName();
            if (commandBus_ != nullptr) {
                auto cmd = std::make_unique<core::commands::ChangeScatterPropertiesCommand>(
                    scene_.get(), idx, dialog.resultColor(),
                    dialog.resultMarkerShape(), dialog.resultMarkerSize(),
                    dialog.resultFilled(), dialog.resultName(),
                    dialog.resultVisible());
                commandBus_->execute(std::move(cmd));
            } else {
                scatter->setColor(dialog.resultColor());
                scatter->setMarkerShape(dialog.resultMarkerShape());
                scatter->setMarkerSize(dialog.resultMarkerSize());
                scatter->setFilled(dialog.resultFilled());
                scatter->setName(dialog.resultName());
                scatter->setVisible(dialog.resultVisible());
            }
            canvas_->update();
        }
        break;
    }
    case plot::PlotItem::Type::Bar: {
        auto* bar = dynamic_cast<plot::BarSeries*>(item);
        if (bar == nullptr) { return; }
        BarPropertyDialog dialog(this);
        dialog.setProperties(bar->fillColor(), bar->outlineColor(),
                             bar->barWidth(), bar->name(), bar->isVisible());
        if (dialog.exec() == QDialog::Accepted) {
            customVisibility_[bar->name()] = dialog.resultVisible();
            customNames_[bar->name()] = dialog.resultName();
            if (commandBus_ != nullptr) {
                auto cmd = std::make_unique<core::commands::ChangeBarPropertiesCommand>(
                    scene_.get(), idx, dialog.resultFillColor(),
                    dialog.resultOutlineColor(), dialog.resultBarWidth(),
                    dialog.resultName(), dialog.resultVisible());
                commandBus_->execute(std::move(cmd));
            } else {
                bar->setFillColor(dialog.resultFillColor());
                bar->setOutlineColor(dialog.resultOutlineColor());
                bar->setBarWidth(dialog.resultBarWidth());
                bar->setName(dialog.resultName());
                bar->setVisible(dialog.resultVisible());
            }
            canvas_->update();
        }
        break;
    }
    case plot::PlotItem::Type::Heatmap: {
        auto* heatmap = dynamic_cast<plot::Heatmap*>(item);
        if (heatmap == nullptr) { return; }
        HeatmapPropertyDialog dialog(this);
        dialog.setProperties(heatmap->colormap(), heatmap->valueMin(),
                             heatmap->valueMax(), false,
                             heatmap->interpolation(), heatmap->opacity(),
                             heatmap->name(), heatmap->isVisible());
        if (dialog.exec() == QDialog::Accepted) {
            customVisibility_[heatmap->name()] = dialog.resultVisible();
            customNames_[heatmap->name()] = dialog.resultName();
            if (commandBus_ != nullptr) {
                auto cmd = std::make_unique<core::commands::ChangeHeatmapPropertiesCommand>(
                    scene_.get(), idx, dialog.resultColormap(),
                    dialog.resultValueMin(), dialog.resultValueMax(),
                    dialog.resultAutoRange(), dialog.resultInterpolation(),
                    dialog.resultOpacity(), dialog.resultName(),
                    dialog.resultVisible());
                commandBus_->execute(std::move(cmd));
            } else {
                heatmap->setColormap(dialog.resultColormap());
                if (dialog.resultAutoRange()) {
                    heatmap->setAutoValueRange();
                } else {
                    heatmap->setValueRange(dialog.resultValueMin(),
                                           dialog.resultValueMax());
                }
                heatmap->setInterpolation(dialog.resultInterpolation());
                heatmap->setOpacity(dialog.resultOpacity());
                heatmap->setName(dialog.resultName());
                heatmap->setVisible(dialog.resultVisible());
            }
            canvas_->update();
        }
        break;
    }
    case plot::PlotItem::Type::Contour: {
        auto* contour = dynamic_cast<plot::ContourPlot*>(item);
        if (contour == nullptr) { return; }
        ContourPropertyDialog dialog(this);
        // Detect auto mode: autoLevelCount_ is private, but we can infer
        // from levels being non-empty (manual) vs providing a count.
        // We pass 0 for auto level count when levels are set manually.
        int autoCount = contour->levels().empty() ? 10 : 0;
        dialog.setProperties(contour->levels(), autoCount,
                             contour->lineColor(), contour->lineWidth(),
                             contour->labelsVisible(), contour->name(),
                             contour->isVisible());
        if (dialog.exec() == QDialog::Accepted) {
            customVisibility_[contour->name()] = dialog.resultVisible();
            customNames_[contour->name()] = dialog.resultName();
            if (commandBus_ != nullptr) {
                auto cmd = std::make_unique<core::commands::ChangeContourPropertiesCommand>(
                    scene_.get(), idx, dialog.resultLevels(),
                    dialog.resultAutoLevelCount(), dialog.resultLineColor(),
                    dialog.resultLineWidth(), dialog.resultLabelsVisible(),
                    dialog.resultName(), dialog.resultVisible());
                commandBus_->execute(std::move(cmd));
            } else {
                if (dialog.resultAutoLevelCount() > 0) {
                    contour->setAutoLevels(dialog.resultAutoLevelCount());
                } else {
                    contour->setLevels(dialog.resultLevels());
                }
                contour->setLineColor(dialog.resultLineColor());
                contour->setLineWidth(dialog.resultLineWidth());
                contour->setLabelsVisible(dialog.resultLabelsVisible());
                contour->setName(dialog.resultName());
                contour->setVisible(dialog.resultVisible());
            }
            canvas_->update();
        }
        break;
    }
    case plot::PlotItem::Type::Histogram: {
        auto* hist = dynamic_cast<plot::HistogramSeries*>(item);
        if (hist == nullptr) { return; }
        HistogramPropertyDialog dialog(this);
        dialog.setProperties(hist->binCount(), hist->binRule(),
                             hist->normalization(), hist->fillColor(),
                             hist->name(), hist->isVisible());
        if (dialog.exec() == QDialog::Accepted) {
            customVisibility_[hist->name()] = dialog.resultVisible();
            customNames_[hist->name()] = dialog.resultName();
            if (commandBus_ != nullptr) {
                auto cmd = std::make_unique<core::commands::ChangeHistogramPropertiesCommand>(
                    scene_.get(), idx, dialog.resultBinCount(),
                    dialog.resultBinRule(), dialog.resultNormalization(),
                    dialog.resultFillColor(), dialog.resultName(),
                    dialog.resultVisible());
                commandBus_->execute(std::move(cmd));
            } else {
                if (dialog.resultBinCount() > 0) {
                    hist->setBinCount(dialog.resultBinCount());
                } else {
                    hist->setAutoBinning(dialog.resultBinRule());
                }
                hist->setNormalization(dialog.resultNormalization());
                hist->setFillColor(dialog.resultFillColor());
                hist->setName(dialog.resultName());
                hist->setVisible(dialog.resultVisible());
            }
            canvas_->update();
        }
        break;
    }
    case plot::PlotItem::Type::BoxPlot: {
        auto* box = dynamic_cast<plot::BoxPlotSeries*>(item);
        if (box == nullptr) { return; }
        BoxPlotPropertyDialog dialog(this);
        dialog.setProperties(box->whiskerRule(), box->notched(),
                             box->outliersVisible(), box->fillColor(),
                             box->name(), box->isVisible());
        if (dialog.exec() == QDialog::Accepted) {
            customVisibility_[box->name()] = dialog.resultVisible();
            customNames_[box->name()] = dialog.resultName();
            if (commandBus_ != nullptr) {
                auto cmd = std::make_unique<core::commands::ChangeBoxPlotPropertiesCommand>(
                    scene_.get(), idx, dialog.resultWhiskerRule(),
                    dialog.resultNotched(), dialog.resultOutliersVisible(),
                    dialog.resultFillColor(), dialog.resultName(),
                    dialog.resultVisible());
                commandBus_->execute(std::move(cmd));
            } else {
                box->setWhiskerRule(dialog.resultWhiskerRule());
                box->setNotched(dialog.resultNotched());
                box->setOutliersVisible(dialog.resultOutliersVisible());
                box->setFillColor(dialog.resultFillColor());
                box->setName(dialog.resultName());
                box->setVisible(dialog.resultVisible());
            }
            canvas_->update();
        }
        break;
    }
    case plot::PlotItem::Type::Violin: {
        auto* violin = dynamic_cast<plot::ViolinSeries*>(item);
        if (violin == nullptr) { return; }
        ViolinPropertyDialog dialog(this);
        dialog.setProperties(violin->kdeBandwidth(), violin->kdeBandwidthAuto(),
                             violin->split(), violin->fillColor(),
                             violin->name(), violin->isVisible());
        if (dialog.exec() == QDialog::Accepted) {
            customVisibility_[violin->name()] = dialog.resultVisible();
            customNames_[violin->name()] = dialog.resultName();
            if (commandBus_ != nullptr) {
                auto cmd = std::make_unique<core::commands::ChangeViolinPropertiesCommand>(
                    scene_.get(), idx, dialog.resultBandwidth(),
                    dialog.resultAutoKde(), dialog.resultSplit(),
                    dialog.resultFillColor(), dialog.resultName(),
                    dialog.resultVisible());
                commandBus_->execute(std::move(cmd));
            } else {
                violin->setKdeBandwidthAuto(dialog.resultAutoKde());
                if (!dialog.resultAutoKde()) {
                    violin->setKdeBandwidth(dialog.resultBandwidth());
                }
                violin->setSplit(dialog.resultSplit());
                violin->setFillColor(dialog.resultFillColor());
                violin->setName(dialog.resultName());
                violin->setVisible(dialog.resultVisible());
            }
            canvas_->update();
        }
        break;
    }
    case plot::PlotItem::Type::Line:
    default: {
        auto& series = scene_->seriesAt(idx);
        LinePropertyDialog dialog(this);
        dialog.setStyle(series.style(), series.name(), series.isVisible());
        if (dialog.exec() == QDialog::Accepted) {
            QString colName = series.name();
            customStyles_[colName] = dialog.resultStyle();
            customVisibility_[colName] = dialog.resultVisible();
            customNames_[colName] = dialog.resultName();
            if (commandBus_ != nullptr) {
                auto cmd = std::make_unique<core::commands::ChangeLineStyleCommand>(
                    scene_.get(), idx, dialog.resultStyle(),
                    dialog.resultName(), dialog.resultVisible());
                commandBus_->execute(std::move(cmd));
            } else {
                series.setStyle(dialog.resultStyle());
                series.setName(dialog.resultName());
                series.setVisible(dialog.resultVisible());
            }
            canvas_->update();
        }
        break;
    }
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
    for (std::size_t i = 0; i < scene_->itemCount(); ++i) {
        const auto* item = scene_->itemAt(i);
        seriesNames.append(item->name());
        seriesColors.append(item->primaryColor());
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

void PlotCanvasDock::setDataFrame(const data::TabularBundle* bundle, const QString& documentPath) {
    documentPath_ = documentPath;
    bundle_ = bundle;

    // Find numeric columns.
    numericColumns_.clear();
    if (bundle != nullptr) {
        for (int i = 0; i < bundle->columnCount(); ++i) {
            auto col = bundle->column(i);
            if (!col) continue;
            // Check if the column is numeric (double or int64)
            bool isNumeric = false;
            try {
                (void)col->doubleData();
                isNumeric = true;
            } catch (...) {}
            if (!isNumeric) {
                try {
                    (void)col->int64Data();
                    isNumeric = true;
                } catch (...) {}
            }
            if (isNumeric) {
                numericColumns_.append(col->name());
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
        delete entry.typeCombo;
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

    // 3D data (has z column) defaults to Scatter; 2D stays Line.
    bool hasZ = numericColumns_.contains(QStringLiteral("z"));
    if (hasZ && !yEntries_.empty() && yEntries_[0].typeCombo != nullptr) {
        yEntries_[0].typeCombo->setCurrentText(QStringLiteral("Scatter"));
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

    // Block signals while setting up so currentIndexChanged doesn't
    // trigger rebuildPlot() before the entry is stored in yEntries_.
    combo->blockSignals(true);
    combo->addItems(numericColumns_);

    // Default to next unused column.
    int defaultIdx = static_cast<int>(yEntries_.size()) + 1;
    if (defaultIdx < combo->count()) {
        combo->setCurrentIndex(defaultIdx);
    }
    combo->blockSignals(false);

    // Per-entry type combo (all plot types).
    auto* typeCombo = new QComboBox(yContainer_);
    typeCombo->addItem(QStringLiteral("Line"));
    typeCombo->addItem(QStringLiteral("Scatter"));
    typeCombo->addItem(QStringLiteral("Bar"));
    typeCombo->addItem(QStringLiteral("Histogram"));
    typeCombo->addItem(QStringLiteral("BoxPlot"));
    typeCombo->addItem(QStringLiteral("Violin"));
    typeCombo->setFixedWidth(90);

    QPushButton* removeBtn = nullptr;
    // Only show remove button for non-first entries.
    if (!yEntries_.empty()) {
        removeBtn = new QPushButton(QStringLiteral("×"), yContainer_);
        removeBtn->setFixedWidth(24);
        removeBtn->setToolTip(tr("Remove this series"));
        connect(removeBtn, &QPushButton::clicked, this, [this, removeBtn]() {
            // Find the entry by its remove button pointer (stable across deletions).
            for (int i = 0; i < static_cast<int>(yEntries_.size()); ++i) {
                if (yEntries_[static_cast<std::size_t>(i)].removeBtn == removeBtn) {
                    removeYSeries(i);
                    return;
                }
            }
        });
    }

    yLayout_->addWidget(typeCombo);
    yLayout_->addWidget(combo);
    if (removeBtn != nullptr) {
        yLayout_->addWidget(removeBtn);
    }

    yEntries_.push_back({combo, typeCombo, removeBtn});

    // Connect AFTER push_back so the entry exists when rebuild runs.
    connect(combo, &QComboBox::currentIndexChanged, this, [this]() {
        rebuildPlot();
    });
    connect(typeCombo, &QComboBox::currentIndexChanged, this, [this]() {
        rebuildPlot();
    });

    // Rebuild to show the new series immediately.
    rebuildPlot();
}

void PlotCanvasDock::removeYSeries(int index) {
    if (index < 0 || index >= static_cast<int>(yEntries_.size())) {
        return;
    }

    auto& entry = yEntries_[static_cast<std::size_t>(index)];
    delete entry.combo;
    delete entry.typeCombo;
    delete entry.removeBtn;
    yEntries_.erase(yEntries_.begin() + index);

    rebuildPlot();
}

void PlotCanvasDock::rebuildPlot() {
    scene_->clearSeries();

    if (bundle_ == nullptr || numericColumns_.isEmpty()) {
        canvas_->update();
        return;
    }

    QString xName = xCombo_->currentText();
    auto xDs = bundle_->columnByName(xName);
    if (!xDs) {
        canvas_->update();
        return;
    }

    // Verify X is double
    bool xIsDouble = false;
    try {
        (void)xDs->doubleData();
        xIsDouble = true;
    } catch (...) {}
    if (!xIsDouble) {
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
        auto yDs = bundle_->columnByName(yName);
        if (!yDs) {
            continue;
        }

        // Y column must be Double
        bool yIsDouble = false;
        try {
            (void)yDs->doubleData();
            yIsDouble = true;
        } catch (...) {}
        if (!yIsDouble) {
            continue;
        }

        // Read plot type from per-entry type combo.
        QString plotType = (entry.typeCombo != nullptr)
                               ? entry.typeCombo->currentText()
                               : QStringLiteral("Line");

        if (plotType == QStringLiteral("Scatter")) {
            auto scatter = std::make_unique<plot::ScatterSeries>(
                xDs, yDs, plot::PlotStyle::fromPalette(seriesIdx).color, yName);
            scene_->addItem(std::move(scatter));
        } else if (plotType == QStringLiteral("Bar")) {
            auto bar = std::make_unique<plot::BarSeries>(
                xDs, yDs, plot::PlotStyle::fromPalette(seriesIdx).color, yName);
            scene_->addItem(std::move(bar));
        } else if (plotType == QStringLiteral("Histogram")) {
            auto hist = std::make_unique<plot::HistogramSeries>(yDs);
            hist->setFillColor(plot::PlotStyle::fromPalette(seriesIdx).color);
            hist->setName(yName);
            scene_->addItem(std::move(hist));
        } else if (plotType == QStringLiteral("BoxPlot")) {
            auto box = std::make_unique<plot::BoxPlotSeries>(yDs);
            box->setFillColor(plot::PlotStyle::fromPalette(seriesIdx).color);
            box->setName(yName);
            scene_->addItem(std::move(box));
        } else if (plotType == QStringLiteral("Violin")) {
            auto violin = std::make_unique<plot::ViolinSeries>(yDs);
            violin->setFillColor(plot::PlotStyle::fromPalette(seriesIdx).color);
            violin->setName(yName);
            scene_->addItem(std::move(violin));
        } else {
            scene_->addSeries(plot::LineSeries(
                xDs, yDs, plot::PlotStyle::fromPalette(seriesIdx), yName));

            // Apply persisted custom style if available (T5).
            auto& addedSeries = scene_->seriesAt(scene_->seriesCount() - 1);
            if (customStyles_.contains(yName)) {
                addedSeries.setStyle(customStyles_[yName]);
            }
        }

        // Apply persisted visibility/name across all types.
        auto* lastItem = scene_->itemAt(scene_->itemCount() - 1);
        if (customVisibility_.contains(yName)) {
            if (auto* ls = dynamic_cast<plot::LineSeries*>(lastItem)) {
                ls->setVisible(customVisibility_[yName]);
            } else if (auto* ss = dynamic_cast<plot::ScatterSeries*>(lastItem)) {
                ss->setVisible(customVisibility_[yName]);
            } else if (auto* bs = dynamic_cast<plot::BarSeries*>(lastItem)) {
                bs->setVisible(customVisibility_[yName]);
            }
        }
        if (customNames_.contains(yName)) {
            if (auto* ls = dynamic_cast<plot::LineSeries*>(lastItem)) {
                ls->setName(customNames_[yName]);
            } else if (auto* ss = dynamic_cast<plot::ScatterSeries*>(lastItem)) {
                ss->setName(customNames_[yName]);
            } else if (auto* bs = dynamic_cast<plot::BarSeries*>(lastItem)) {
                bs->setName(customNames_[yName]);
            }
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
