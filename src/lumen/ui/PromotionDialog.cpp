#include "PromotionDialog.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

namespace lumen::ui {

PromotionDialog::PromotionDialog(const QString& elementName,
                                  const QString& themeName,
                                  bool themeIsBuiltin,
                                  QWidget* parent)
    : QDialog(parent)
    , themeIsBuiltin_(themeIsBuiltin)
    , themeName_(themeName)
{
    setWindowTitle(tr("Apply Style Change"));
    setMinimumWidth(350);

    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(
        tr("Editing: %1").arg(elementName), this));
    layout->addSpacing(8);

    layout->addWidget(new QLabel(tr("Apply to:"), this));

    auto* group = new QButtonGroup(this);

    elementRadio_ = new QRadioButton(tr("This element only"), this);
    elementRadio_->setChecked(true);
    group->addButton(elementRadio_);
    layout->addWidget(elementRadio_);

    plotRadio_ = new QRadioButton(tr("All matching in this plot"), this);
    group->addButton(plotRadio_);
    layout->addWidget(plotRadio_);

    themeRadio_ = new QRadioButton(
        tr("Save to current theme (%1)").arg(themeName), this);
    group->addButton(themeRadio_);
    layout->addWidget(themeRadio_);

    if (themeIsBuiltin) {
        connect(themeRadio_, &QRadioButton::toggled,
                this, &PromotionDialog::onThemeRadioToggled);
    }

    layout->addSpacing(12);

    buttonBox_ = new QDialogButtonBox(
        QDialogButtonBox::Cancel | QDialogButtonBox::Ok, this);
    buttonBox_->button(QDialogButtonBox::Ok)->setText(tr("Apply"));
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox_);
}

style::CascadeLevel PromotionDialog::selectedLevel() const
{
    if (themeRadio_->isChecked())
        return style::CascadeLevel::Theme;
    if (plotRadio_->isChecked())
        return style::CascadeLevel::PlotInstance;
    return style::CascadeLevel::ElementOverride;
}

void PromotionDialog::onThemeRadioToggled(bool checked)
{
    if (!checked || !themeIsBuiltin_)
        return;

    auto result = QMessageBox::question(
        this, tr("Built-in Theme"),
        tr("Built-in themes cannot be modified.\nSave as new theme?"),
        QMessageBox::Yes | QMessageBox::Cancel);

    if (result == QMessageBox::Yes) {
        bool ok = false;
        QString name = QInputDialog::getText(
            this, tr("New Theme Name"),
            tr("Theme name:"), QLineEdit::Normal,
            themeName_ + QStringLiteral("-custom"), &ok);
        if (ok && !name.isEmpty()) {
            wantsFork_ = true;
            forkName_ = name;
        } else {
            elementRadio_->setChecked(true);
        }
    } else {
        elementRadio_->setChecked(true);
    }
}

}  // namespace lumen::ui
