#include "BarPropertyDialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

namespace lumen::ui {

BarPropertyDialog::BarPropertyDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Bar Properties"));

    auto* layout = new QFormLayout(this);

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

    outlineColorButton_ = new QPushButton(this);
    outlineColorButton_->setFixedSize(60, 24);
    connect(outlineColorButton_, &QPushButton::clicked, this, [this]() {
        QColor chosen = QColorDialog::getColor(currentOutlineColor_, this,
                                               tr("Select Outline Color"));
        if (chosen.isValid()) {
            currentOutlineColor_ = chosen;
            updateOutlineColorButton();
        }
    });
    layout->addRow(tr("Outline color:"), outlineColorButton_);

    outlineNoneCheck_ = new QCheckBox(tr("No outline"), this);
    connect(outlineNoneCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        outlineColorButton_->setEnabled(!checked);
    });
    layout->addRow(QString(), outlineNoneCheck_);

    barWidthSpin_ = new QDoubleSpinBox(this);
    barWidthSpin_->setRange(0.1, 1.0);
    barWidthSpin_->setSingleStep(0.1);
    barWidthSpin_->setDecimals(1);
    layout->addRow(tr("Bar width:"), barWidthSpin_);

    nameEdit_ = new QLineEdit(this);
    layout->addRow(tr("Name:"), nameEdit_);

    visibleCheck_ = new QCheckBox(tr("Visible"), this);
    layout->addRow(QString(), visibleCheck_);

    buttonBox_ = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addRow(buttonBox_);
}

void BarPropertyDialog::setProperties(QColor fillColor, QColor outlineColor,
                                      double barWidth, const QString& name,
                                      bool visible) {
    currentFillColor_ = std::move(fillColor);
    updateFillColorButton();

    currentOutlineColor_ = std::move(outlineColor);
    bool noOutline = (currentOutlineColor_.alpha() == 0);
    outlineNoneCheck_->setChecked(noOutline);
    outlineColorButton_->setEnabled(!noOutline);
    updateOutlineColorButton();

    barWidthSpin_->setValue(barWidth);
    nameEdit_->setText(name);
    visibleCheck_->setChecked(visible);
}

QColor BarPropertyDialog::resultOutlineColor() const {
    if (outlineNoneCheck_->isChecked()) {
        return Qt::transparent;
    }
    return currentOutlineColor_;
}

double BarPropertyDialog::resultBarWidth() const {
    return barWidthSpin_->value();
}

QString BarPropertyDialog::resultName() const {
    return nameEdit_->text();
}

bool BarPropertyDialog::resultVisible() const {
    return visibleCheck_->isChecked();
}

void BarPropertyDialog::updateFillColorButton() {
    fillColorButton_->setStyleSheet(
        QStringLiteral("background-color: %1; border: 1px solid #888;")
            .arg(currentFillColor_.name()));
}

void BarPropertyDialog::updateOutlineColorButton() {
    if (currentOutlineColor_.alpha() == 0) {
        outlineColorButton_->setStyleSheet(
            QStringLiteral("background-color: #fff; border: 1px dashed #888;"));
    } else {
        outlineColorButton_->setStyleSheet(
            QStringLiteral("background-color: %1; border: 1px solid #888;")
                .arg(currentOutlineColor_.name()));
    }
}

}  // namespace lumen::ui
