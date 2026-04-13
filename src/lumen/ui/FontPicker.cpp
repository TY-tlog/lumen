#include "FontPicker.h"

#include <QComboBox>
#include <QFont>
#include <QFontDialog>
#include <QHBoxLayout>
#include <QLabel>

namespace lumen::ui {

FontPicker::FontPicker(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    combo_ = new QComboBox(this);
    combo_->addItem(QStringLiteral("Liberation Serif"));
    combo_->addItem(QStringLiteral("Liberation Sans"));
    combo_->addItem(QStringLiteral("Computer Modern"));
    combo_->addItem(QStringLiteral("Source Serif Pro"));
    combo_->insertSeparator(4);
    combo_->addItem(QStringLiteral("System..."));

    preview_ = new QLabel(QStringLiteral("AaBbCc 123"), this);
    preview_->setMinimumWidth(100);

    layout->addWidget(combo_, 1);
    layout->addWidget(preview_);

    connect(combo_, &QComboBox::currentIndexChanged,
            this, &FontPicker::onComboChanged);

    onComboChanged(0);
}

QString FontPicker::selectedFamily() const
{
    return combo_->currentText();
}

void FontPicker::onComboChanged(int index)
{
    if (combo_->currentText() == QStringLiteral("System...")) {
        bool ok = false;
        QFont font = QFontDialog::getFont(&ok, this);
        if (ok) {
            // Insert the system font before the separator.
            combo_->blockSignals(true);
            combo_->insertItem(4, font.family());
            combo_->setCurrentIndex(4);
            combo_->blockSignals(false);
        } else {
            combo_->setCurrentIndex(0);
            return;
        }
    }

    QFont previewFont(combo_->currentText());
    previewFont.setPixelSize(13);
    preview_->setFont(previewFont);

    Q_UNUSED(index)
    emit fontChanged(combo_->currentText());
}

}  // namespace lumen::ui
