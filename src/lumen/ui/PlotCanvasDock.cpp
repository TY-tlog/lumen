#include "PlotCanvasDock.h"

#include "PlotCanvas.h"

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

void PlotCanvasDock::setDataFrame(const data::DataFrame* df) {
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
