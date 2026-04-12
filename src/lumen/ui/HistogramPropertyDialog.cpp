#include "HistogramPropertyDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QColorDialog>
#include <QSpinBox>

namespace lumen::ui {

HistogramPropertyDialog::HistogramPropertyDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Histogram Properties"));

    auto* layout = new QFormLayout(this);

    // Auto binning checkbox.
    autoBinCheck_ = new QCheckBox(tr("Auto binning"), this);
    layout->addRow(QString(), autoBinCheck_);

    // Bin count (manual).
    binCountSpin_ = new QSpinBox(this);
    binCountSpin_->setRange(1, 1000);
    binCountSpin_->setValue(20);
    layout->addRow(tr("Bin count:"), binCountSpin_);

    // Bin rule.
    binRuleCombo_ = new QComboBox(this);
    binRuleCombo_->addItem(tr("Scott"),
        static_cast<int>(plot::HistogramSeries::BinRule::Scott));
    binRuleCombo_->addItem(tr("Freedman-Diaconis"),
        static_cast<int>(plot::HistogramSeries::BinRule::FreedmanDiaconis));
    binRuleCombo_->addItem(tr("Sturges"),
        static_cast<int>(plot::HistogramSeries::BinRule::Sturges));
    layout->addRow(tr("Bin rule:"), binRuleCombo_);

    connect(autoBinCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        binCountSpin_->setEnabled(!checked);
        binRuleCombo_->setEnabled(checked);
    });

    // Normalization.
    normCombo_ = new QComboBox(this);
    normCombo_->addItem(tr("Count"),
        static_cast<int>(plot::HistogramSeries::Normalization::Count));
    normCombo_->addItem(tr("Density"),
        static_cast<int>(plot::HistogramSeries::Normalization::Density));
    normCombo_->addItem(tr("Probability"),
        static_cast<int>(plot::HistogramSeries::Normalization::Probability));
    layout->addRow(tr("Normalization:"), normCombo_);

    // Fill color.
    fillColorButton_ = new QPushButton(this);
    fillColorButton_->setFixedSize(60, 24);
    connect(fillColorButton_, &QPushButton::clicked, this, [this]() {
        QColor chosen = QColorDialog::getColor(currentFillColor_, this,
                                               tr("Select Fill Color"));
        if (chosen.isValid()) {
            currentFillColor_ = chosen;
            updateFillColorButton();
        }
    });
    layout->addRow(tr("Fill color:"), fillColorButton_);

    // Name.
    nameEdit_ = new QLineEdit(this);
    layout->addRow(tr("Name:"), nameEdit_);

    // Visible.
    visibleCheck_ = new QCheckBox(tr("Visible"), this);
    layout->addRow(QString(), visibleCheck_);

    // OK / Cancel.
    buttonBox_ = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addRow(buttonBox_);
}

void HistogramPropertyDialog::setProperties(
    int binCount, plot::HistogramSeries::BinRule binRule,
    plot::HistogramSeries::Normalization normalization,
    QColor fillColor, const QString& name, bool visible) {
    bool isAuto = (binCount == 0);
    autoBinCheck_->setChecked(isAuto);
    binCountSpin_->setEnabled(!isAuto);
    binRuleCombo_->setEnabled(isAuto);

    if (binCount > 0) {
        binCountSpin_->setValue(binCount);
    }

    int ruleIdx = binRuleCombo_->findData(static_cast<int>(binRule));
    if (ruleIdx >= 0) {
        binRuleCombo_->setCurrentIndex(ruleIdx);
    }

    int normIdx = normCombo_->findData(static_cast<int>(normalization));
    if (normIdx >= 0) {
        normCombo_->setCurrentIndex(normIdx);
    }

    currentFillColor_ = std::move(fillColor);
    updateFillColorButton();
    nameEdit_->setText(name);
    visibleCheck_->setChecked(visible);
}

int HistogramPropertyDialog::resultBinCount() const {
    if (autoBinCheck_->isChecked()) {
        return 0;  // Auto binning: caller uses bin rule.
    }
    return binCountSpin_->value();
}

plot::HistogramSeries::BinRule HistogramPropertyDialog::resultBinRule() const {
    return static_cast<plot::HistogramSeries::BinRule>(
        binRuleCombo_->currentData().toInt());
}

plot::HistogramSeries::Normalization HistogramPropertyDialog::resultNormalization() const {
    return static_cast<plot::HistogramSeries::Normalization>(
        normCombo_->currentData().toInt());
}

QString HistogramPropertyDialog::resultName() const {
    return nameEdit_->text();
}

bool HistogramPropertyDialog::resultVisible() const {
    return visibleCheck_->isChecked();
}

void HistogramPropertyDialog::updateFillColorButton() {
    fillColorButton_->setStyleSheet(
        QStringLiteral("background-color: %1; border: 1px solid #888;")
            .arg(currentFillColor_.name()));
}

}  // namespace lumen::ui
