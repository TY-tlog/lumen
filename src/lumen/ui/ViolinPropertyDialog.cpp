#include "ViolinPropertyDialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

namespace lumen::ui {

ViolinPropertyDialog::ViolinPropertyDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Violin Properties"));

    auto* layout = new QFormLayout(this);

    // Auto KDE bandwidth.
    autoKdeCheck_ = new QCheckBox(tr("Auto bandwidth (Silverman)"), this);
    layout->addRow(QString(), autoKdeCheck_);

    // Bandwidth.
    bandwidthSpin_ = new QDoubleSpinBox(this);
    bandwidthSpin_->setRange(0.001, 100.0);
    bandwidthSpin_->setSingleStep(0.1);
    bandwidthSpin_->setDecimals(3);
    layout->addRow(tr("Bandwidth:"), bandwidthSpin_);

    connect(autoKdeCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        bandwidthSpin_->setEnabled(!checked);
    });

    // Split.
    splitCheck_ = new QCheckBox(tr("Split (half violin)"), this);
    layout->addRow(QString(), splitCheck_);

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

void ViolinPropertyDialog::setProperties(
    double bandwidth, bool autoKde, bool split, QColor fillColor,
    const QString& name, bool visible) {
    autoKdeCheck_->setChecked(autoKde);
    bandwidthSpin_->setValue(bandwidth);
    bandwidthSpin_->setEnabled(!autoKde);
    splitCheck_->setChecked(split);
    currentFillColor_ = std::move(fillColor);
    updateFillColorButton();
    nameEdit_->setText(name);
    visibleCheck_->setChecked(visible);
}

double ViolinPropertyDialog::resultBandwidth() const {
    return bandwidthSpin_->value();
}

bool ViolinPropertyDialog::resultAutoKde() const {
    return autoKdeCheck_->isChecked();
}

bool ViolinPropertyDialog::resultSplit() const {
    return splitCheck_->isChecked();
}

QString ViolinPropertyDialog::resultName() const {
    return nameEdit_->text();
}

bool ViolinPropertyDialog::resultVisible() const {
    return visibleCheck_->isChecked();
}

void ViolinPropertyDialog::updateFillColorButton() {
    fillColorButton_->setStyleSheet(
        QStringLiteral("background-color: %1; border: 1px solid #888;")
            .arg(currentFillColor_.name()));
}

}  // namespace lumen::ui
