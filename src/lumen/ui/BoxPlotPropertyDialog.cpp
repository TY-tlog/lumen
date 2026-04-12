#include "BoxPlotPropertyDialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

namespace lumen::ui {

BoxPlotPropertyDialog::BoxPlotPropertyDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Box Plot Properties"));

    auto* layout = new QFormLayout(this);

    // Whisker rule.
    whiskerRuleCombo_ = new QComboBox(this);
    whiskerRuleCombo_->addItem(tr("Tukey (1.5 IQR)"),
        static_cast<int>(plot::BoxPlotSeries::WhiskerRule::Tukey));
    whiskerRuleCombo_->addItem(tr("Min/Max"),
        static_cast<int>(plot::BoxPlotSeries::WhiskerRule::MinMax));
    whiskerRuleCombo_->addItem(tr("Percentile (5th/95th)"),
        static_cast<int>(plot::BoxPlotSeries::WhiskerRule::Percentile));
    layout->addRow(tr("Whisker rule:"), whiskerRuleCombo_);

    // Notched.
    notchedCheck_ = new QCheckBox(tr("Notched"), this);
    layout->addRow(QString(), notchedCheck_);

    // Outliers visible.
    outliersCheck_ = new QCheckBox(tr("Show outliers"), this);
    layout->addRow(QString(), outliersCheck_);

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

void BoxPlotPropertyDialog::setProperties(
    plot::BoxPlotSeries::WhiskerRule whiskerRule, bool notched,
    bool outliersVisible, QColor fillColor, const QString& name,
    bool visible) {
    int ruleIdx = whiskerRuleCombo_->findData(static_cast<int>(whiskerRule));
    if (ruleIdx >= 0) {
        whiskerRuleCombo_->setCurrentIndex(ruleIdx);
    }
    notchedCheck_->setChecked(notched);
    outliersCheck_->setChecked(outliersVisible);
    currentFillColor_ = std::move(fillColor);
    updateFillColorButton();
    nameEdit_->setText(name);
    visibleCheck_->setChecked(visible);
}

plot::BoxPlotSeries::WhiskerRule BoxPlotPropertyDialog::resultWhiskerRule() const {
    return static_cast<plot::BoxPlotSeries::WhiskerRule>(
        whiskerRuleCombo_->currentData().toInt());
}

bool BoxPlotPropertyDialog::resultNotched() const {
    return notchedCheck_->isChecked();
}

bool BoxPlotPropertyDialog::resultOutliersVisible() const {
    return outliersCheck_->isChecked();
}

QString BoxPlotPropertyDialog::resultName() const {
    return nameEdit_->text();
}

bool BoxPlotPropertyDialog::resultVisible() const {
    return visibleCheck_->isChecked();
}

void BoxPlotPropertyDialog::updateFillColorButton() {
    fillColorButton_->setStyleSheet(
        QStringLiteral("background-color: %1; border: 1px solid #888;")
            .arg(currentFillColor_.name()));
}

}  // namespace lumen::ui
