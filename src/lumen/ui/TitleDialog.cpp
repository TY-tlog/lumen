#include "TitleDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>

namespace lumen::ui {

TitleDialog::TitleDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Title Properties"));

    auto* layout = new QFormLayout(this);

    // Font size.
    fontPxSpin_ = new QSpinBox(this);
    fontPxSpin_->setRange(8, 72);
    fontPxSpin_->setSuffix(QStringLiteral(" px"));
    layout->addRow(tr("Font size:"), fontPxSpin_);

    // Weight.
    weightCombo_ = new QComboBox(this);
    weightCombo_->addItem(tr("Normal"), static_cast<int>(QFont::Normal));
    weightCombo_->addItem(tr("Medium"), static_cast<int>(QFont::Medium));
    weightCombo_->addItem(tr("DemiBold"), static_cast<int>(QFont::DemiBold));
    weightCombo_->addItem(tr("Bold"), static_cast<int>(QFont::Bold));
    layout->addRow(tr("Weight:"), weightCombo_);

    // OK / Cancel.
    buttonBox_ = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addRow(buttonBox_);
}

void TitleDialog::setTitleProperties(int fontPx, QFont::Weight weight) {
    fontPxSpin_->setValue(fontPx);

    int idx = weightCombo_->findData(static_cast<int>(weight));
    if (idx >= 0) {
        weightCombo_->setCurrentIndex(idx);
    }
}

int TitleDialog::resultFontPx() const {
    return fontPxSpin_->value();
}

QFont::Weight TitleDialog::resultWeight() const {
    return static_cast<QFont::Weight>(weightCombo_->currentData().toInt());
}

}  // namespace lumen::ui
