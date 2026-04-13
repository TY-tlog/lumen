#include "AxisDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>

namespace lumen::ui {

AxisDialog::AxisDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Axis Properties"));

    auto* layout = new QFormLayout(this);

    // Label.
    labelEdit_ = new QLineEdit(this);
    layout->addRow(tr("Label:"), labelEdit_);

    // Range mode.
    rangeModeCombo_ = new QComboBox(this);
    rangeModeCombo_->addItem(tr("Auto"), static_cast<int>(plot::RangeMode::Auto));
    rangeModeCombo_->addItem(tr("Manual"), static_cast<int>(plot::RangeMode::Manual));
    connect(rangeModeCombo_, &QComboBox::currentIndexChanged,
            this, &AxisDialog::updateRangeModeVisibility);
    layout->addRow(tr("Range mode:"), rangeModeCombo_);

    // Manual min.
    manualMinSpin_ = new QDoubleSpinBox(this);
    manualMinSpin_->setRange(-1e15, 1e15);
    manualMinSpin_->setDecimals(6);
    layout->addRow(tr("Min:"), manualMinSpin_);

    // Manual max.
    manualMaxSpin_ = new QDoubleSpinBox(this);
    manualMaxSpin_->setRange(-1e15, 1e15);
    manualMaxSpin_->setDecimals(6);
    layout->addRow(tr("Max:"), manualMaxSpin_);

    // Tick count.
    tickCountSpin_ = new QSpinBox(this);
    tickCountSpin_->setRange(0, 20);
    tickCountSpin_->setSpecialValueText(tr("Auto"));
    layout->addRow(tr("Tick count:"), tickCountSpin_);

    // Tick format.
    tickFormatCombo_ = new QComboBox(this);
    tickFormatCombo_->addItem(tr("Auto"), static_cast<int>(plot::TickFormat::Auto));
    tickFormatCombo_->addItem(tr("Scientific"), static_cast<int>(plot::TickFormat::Scientific));
    tickFormatCombo_->addItem(tr("Fixed"), static_cast<int>(plot::TickFormat::Fixed));
    connect(tickFormatCombo_, &QComboBox::currentIndexChanged,
            this, &AxisDialog::updateTickFormatVisibility);
    layout->addRow(tr("Tick format:"), tickFormatCombo_);

    // Tick format decimals.
    tickFormatDecimalsSpin_ = new QSpinBox(this);
    tickFormatDecimalsSpin_->setRange(0, 6);
    layout->addRow(tr("Decimals:"), tickFormatDecimalsSpin_);

    // Grid visible.
    gridVisibleCheck_ = new QCheckBox(tr("Show grid lines"), this);
    layout->addRow(QString(), gridVisibleCheck_);

    latexCheck_ = new QCheckBox(tr("LaTeX math mode"), this);
    layout->addRow(QString(), latexCheck_);

    // OK / Cancel.
    buttonBox_ = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addRow(buttonBox_);

    // Set initial visibility.
    updateRangeModeVisibility();
    updateTickFormatVisibility();
}

void AxisDialog::setAxisProperties(const QString& label, plot::RangeMode rangeMode,
                                   double manualMin, double manualMax,
                                   int tickCount, plot::TickFormat tickFormat,
                                   int tickFormatDecimals, bool gridVisible) {
    labelEdit_->setText(label);

    int rmIdx = rangeModeCombo_->findData(static_cast<int>(rangeMode));
    if (rmIdx >= 0) {
        rangeModeCombo_->setCurrentIndex(rmIdx);
    }

    manualMinSpin_->setValue(manualMin);
    manualMaxSpin_->setValue(manualMax);
    tickCountSpin_->setValue(tickCount);

    int tfIdx = tickFormatCombo_->findData(static_cast<int>(tickFormat));
    if (tfIdx >= 0) {
        tickFormatCombo_->setCurrentIndex(tfIdx);
    }

    tickFormatDecimalsSpin_->setValue(tickFormatDecimals);
    gridVisibleCheck_->setChecked(gridVisible);

    updateRangeModeVisibility();
    updateTickFormatVisibility();
}

QString AxisDialog::resultLabel() const {
    return labelEdit_->text();
}

plot::RangeMode AxisDialog::resultRangeMode() const {
    return static_cast<plot::RangeMode>(rangeModeCombo_->currentData().toInt());
}

double AxisDialog::resultManualMin() const {
    return manualMinSpin_->value();
}

double AxisDialog::resultManualMax() const {
    return manualMaxSpin_->value();
}

int AxisDialog::resultTickCount() const {
    return tickCountSpin_->value();
}

plot::TickFormat AxisDialog::resultTickFormat() const {
    return static_cast<plot::TickFormat>(tickFormatCombo_->currentData().toInt());
}

int AxisDialog::resultTickFormatDecimals() const {
    return tickFormatDecimalsSpin_->value();
}

bool AxisDialog::resultGridVisible() const {
    return gridVisibleCheck_->isChecked();
}

bool AxisDialog::resultLatexMode() const {
    return latexCheck_->isChecked();
}

void AxisDialog::updateRangeModeVisibility() {
    bool manual = resultRangeMode() == plot::RangeMode::Manual;
    manualMinSpin_->setVisible(manual);
    manualMaxSpin_->setVisible(manual);
}

void AxisDialog::updateTickFormatVisibility() {
    bool fixed = resultTickFormat() == plot::TickFormat::Fixed;
    tickFormatDecimalsSpin_->setVisible(fixed);
}

}  // namespace lumen::ui
