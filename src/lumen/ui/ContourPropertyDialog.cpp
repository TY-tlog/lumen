#include "ContourPropertyDialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

#include <algorithm>

namespace lumen::ui {

ContourPropertyDialog::ContourPropertyDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Contour Properties"));

    auto* layout = new QFormLayout(this);

    // Auto levels.
    autoLevelsCheck_ = new QCheckBox(tr("Auto levels"), this);
    layout->addRow(QString(), autoLevelsCheck_);

    levelCountSpin_ = new QSpinBox(this);
    levelCountSpin_->setRange(2, 100);
    levelCountSpin_->setValue(10);
    layout->addRow(tr("Level count:"), levelCountSpin_);

    // Manual levels (comma-separated).
    manualLevelsEdit_ = new QLineEdit(this);
    manualLevelsEdit_->setPlaceholderText(tr("e.g. 0.1, 0.5, 1.0, 2.0"));
    layout->addRow(tr("Manual levels:"), manualLevelsEdit_);

    connect(autoLevelsCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        levelCountSpin_->setEnabled(checked);
        manualLevelsEdit_->setEnabled(!checked);
    });

    // Line color.
    lineColorButton_ = new QPushButton(this);
    lineColorButton_->setFixedSize(60, 24);
    connect(lineColorButton_, &QPushButton::clicked, this, [this]() {
        QColor chosen = QColorDialog::getColor(currentLineColor_, this,
                                               tr("Select Line Color"));
        if (chosen.isValid()) {
            currentLineColor_ = chosen;
            updateColorButton();
        }
    });
    layout->addRow(tr("Line color:"), lineColorButton_);

    // Line width.
    lineWidthSpin_ = new QDoubleSpinBox(this);
    lineWidthSpin_->setRange(0.5, 10.0);
    lineWidthSpin_->setSingleStep(0.5);
    lineWidthSpin_->setSuffix(QStringLiteral(" px"));
    lineWidthSpin_->setDecimals(1);
    layout->addRow(tr("Line width:"), lineWidthSpin_);

    // Labels visible.
    labelsCheck_ = new QCheckBox(tr("Show labels"), this);
    layout->addRow(QString(), labelsCheck_);

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

void ContourPropertyDialog::setProperties(
    const std::vector<double>& levels, int autoLevelCount,
    QColor lineColor, double lineWidth, bool labelsVisible,
    const QString& name, bool visible) {
    bool isAuto = (autoLevelCount > 0);
    autoLevelsCheck_->setChecked(isAuto);
    levelCountSpin_->setValue(isAuto ? autoLevelCount : 10);
    levelCountSpin_->setEnabled(isAuto);
    manualLevelsEdit_->setEnabled(!isAuto);

    // Format levels as comma-separated string.
    QStringList parts;
    for (double v : levels) {
        parts.append(QString::number(v, 'g', 6));
    }
    manualLevelsEdit_->setText(parts.join(QStringLiteral(", ")));

    currentLineColor_ = std::move(lineColor);
    updateColorButton();
    lineWidthSpin_->setValue(lineWidth);
    labelsCheck_->setChecked(labelsVisible);
    nameEdit_->setText(name);
    visibleCheck_->setChecked(visible);
}

std::vector<double> ContourPropertyDialog::resultLevels() const {
    std::vector<double> result;
    if (autoLevelsCheck_->isChecked()) {
        return result;  // Empty: caller should use auto level count.
    }
    QString text = manualLevelsEdit_->text();
    QStringList parts = text.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        bool ok = false;
        double v = part.trimmed().toDouble(&ok);
        if (ok) {
            result.push_back(v);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

int ContourPropertyDialog::resultAutoLevelCount() const {
    if (autoLevelsCheck_->isChecked()) {
        return levelCountSpin_->value();
    }
    return 0;  // Manual mode.
}

double ContourPropertyDialog::resultLineWidth() const {
    return lineWidthSpin_->value();
}

bool ContourPropertyDialog::resultLabelsVisible() const {
    return labelsCheck_->isChecked();
}

QString ContourPropertyDialog::resultName() const {
    return nameEdit_->text();
}

bool ContourPropertyDialog::resultVisible() const {
    return visibleCheck_->isChecked();
}

void ContourPropertyDialog::updateColorButton() {
    lineColorButton_->setStyleSheet(
        QStringLiteral("background-color: %1; border: 1px solid #888;")
            .arg(currentLineColor_.name()));
}

}  // namespace lumen::ui
