#include "LinePropertyDialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

namespace lumen::ui {

LinePropertyDialog::LinePropertyDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Line Properties"));

    auto* layout = new QFormLayout(this);

    // Color button.
    colorButton_ = new QPushButton(this);
    colorButton_->setFixedSize(60, 24);
    connect(colorButton_, &QPushButton::clicked, this, [this]() {
        QColor chosen = QColorDialog::getColor(currentColor_, this, tr("Select Line Color"));
        if (chosen.isValid()) {
            currentColor_ = chosen;
            updateColorButton();
        }
    });
    layout->addRow(tr("Color:"), colorButton_);

    // Line width.
    widthSpin_ = new QDoubleSpinBox(this);
    widthSpin_->setRange(0.5, 10.0);
    widthSpin_->setSingleStep(0.5);
    widthSpin_->setSuffix(QStringLiteral(" px"));
    widthSpin_->setDecimals(1);
    layout->addRow(tr("Line width:"), widthSpin_);

    // Line style.
    styleCombo_ = new QComboBox(this);
    styleCombo_->addItem(tr("Solid"),     static_cast<int>(Qt::SolidLine));
    styleCombo_->addItem(tr("Dash"),      static_cast<int>(Qt::DashLine));
    styleCombo_->addItem(tr("Dot"),       static_cast<int>(Qt::DotLine));
    styleCombo_->addItem(tr("DashDot"),   static_cast<int>(Qt::DashDotLine));
    styleCombo_->addItem(tr("DashDotDot"),static_cast<int>(Qt::DashDotDotLine));
    layout->addRow(tr("Line style:"), styleCombo_);

    // Series name.
    nameEdit_ = new QLineEdit(this);
    layout->addRow(tr("Name:"), nameEdit_);

    // Visible checkbox.
    visibleCheck_ = new QCheckBox(tr("Visible"), this);
    layout->addRow(QString(), visibleCheck_);

    // OK / Cancel.
    buttonBox_ = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addRow(buttonBox_);
}

void LinePropertyDialog::setStyle(const plot::PlotStyle& style,
                                  const QString& name, bool visible) {
    currentColor_ = style.color;
    updateColorButton();

    widthSpin_->setValue(style.lineWidth);

    // Find matching pen style in combo.
    int penIdx = styleCombo_->findData(static_cast<int>(style.penStyle));
    if (penIdx >= 0) {
        styleCombo_->setCurrentIndex(penIdx);
    }

    nameEdit_->setText(name);
    visibleCheck_->setChecked(visible);
}

plot::PlotStyle LinePropertyDialog::resultStyle() const {
    plot::PlotStyle s;
    s.color = currentColor_;
    s.lineWidth = widthSpin_->value();
    s.penStyle = static_cast<Qt::PenStyle>(
        styleCombo_->currentData().toInt());
    return s;
}

QString LinePropertyDialog::resultName() const {
    return nameEdit_->text();
}

bool LinePropertyDialog::resultVisible() const {
    return visibleCheck_->isChecked();
}

void LinePropertyDialog::updateColorButton() {
    colorButton_->setStyleSheet(
        QStringLiteral("background-color: %1; border: 1px solid #888;")
            .arg(currentColor_.name()));
}

}  // namespace lumen::ui
