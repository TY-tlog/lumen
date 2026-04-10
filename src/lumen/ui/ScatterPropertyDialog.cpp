#include "ScatterPropertyDialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

namespace lumen::ui {

ScatterPropertyDialog::ScatterPropertyDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Scatter Properties"));

    auto* layout = new QFormLayout(this);

    colorButton_ = new QPushButton(this);
    colorButton_->setFixedSize(60, 24);
    connect(colorButton_, &QPushButton::clicked, this, [this]() {
        QColor chosen = QColorDialog::getColor(currentColor_, this,
                                               tr("Select Marker Color"));
        if (chosen.isValid()) {
            currentColor_ = chosen;
            updateColorButton();
        }
    });
    layout->addRow(tr("Color:"), colorButton_);

    shapeCombo_ = new QComboBox(this);
    shapeCombo_->addItem(tr("Circle"),   static_cast<int>(plot::MarkerShape::Circle));
    shapeCombo_->addItem(tr("Square"),   static_cast<int>(plot::MarkerShape::Square));
    shapeCombo_->addItem(tr("Triangle"), static_cast<int>(plot::MarkerShape::Triangle));
    shapeCombo_->addItem(tr("Diamond"),  static_cast<int>(plot::MarkerShape::Diamond));
    shapeCombo_->addItem(tr("Plus"),     static_cast<int>(plot::MarkerShape::Plus));
    shapeCombo_->addItem(tr("Cross"),    static_cast<int>(plot::MarkerShape::Cross));
    layout->addRow(tr("Marker shape:"), shapeCombo_);

    sizeSpin_ = new QSpinBox(this);
    sizeSpin_->setRange(3, 20);
    sizeSpin_->setSuffix(QStringLiteral(" px"));
    layout->addRow(tr("Marker size:"), sizeSpin_);

    filledCheck_ = new QCheckBox(tr("Filled"), this);
    layout->addRow(QString(), filledCheck_);

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

void ScatterPropertyDialog::setProperties(QColor color,
                                          plot::MarkerShape shape, int size,
                                          bool filled, const QString& name,
                                          bool visible) {
    currentColor_ = std::move(color);
    updateColorButton();

    int shapeIdx = shapeCombo_->findData(static_cast<int>(shape));
    if (shapeIdx >= 0) {
        shapeCombo_->setCurrentIndex(shapeIdx);
    }

    sizeSpin_->setValue(size);
    filledCheck_->setChecked(filled);
    nameEdit_->setText(name);
    visibleCheck_->setChecked(visible);
}

plot::MarkerShape ScatterPropertyDialog::resultMarkerShape() const {
    return static_cast<plot::MarkerShape>(shapeCombo_->currentData().toInt());
}

int ScatterPropertyDialog::resultMarkerSize() const {
    return sizeSpin_->value();
}

bool ScatterPropertyDialog::resultFilled() const {
    return filledCheck_->isChecked();
}

QString ScatterPropertyDialog::resultName() const {
    return nameEdit_->text();
}

bool ScatterPropertyDialog::resultVisible() const {
    return visibleCheck_->isChecked();
}

void ScatterPropertyDialog::updateColorButton() {
    colorButton_->setStyleSheet(
        QStringLiteral("background-color: %1; border: 1px solid #888;")
            .arg(currentColor_.name()));
}

}  // namespace lumen::ui
