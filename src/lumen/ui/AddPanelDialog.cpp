#include "AddPanelDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>

namespace lumen::ui {

AddPanelDialog::AddPanelDialog(int maxRows, int maxCols, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Add Panel"));

    auto* layout = new QFormLayout(this);

    titleEdit_ = new QLineEdit(this);
    titleEdit_->setPlaceholderText(tr("Panel title (optional)"));
    layout->addRow(tr("Title:"), titleEdit_);

    rowSpin_ = new QSpinBox(this);
    rowSpin_->setRange(0, maxRows - 1);
    layout->addRow(tr("Row:"), rowSpin_);

    colSpin_ = new QSpinBox(this);
    colSpin_->setRange(0, maxCols - 1);
    layout->addRow(tr("Column:"), colSpin_);

    rowSpanSpin_ = new QSpinBox(this);
    rowSpanSpin_->setRange(1, maxRows);
    rowSpanSpin_->setValue(1);
    layout->addRow(tr("Row span:"), rowSpanSpin_);

    colSpanSpin_ = new QSpinBox(this);
    colSpanSpin_->setRange(1, maxCols);
    colSpanSpin_->setValue(1);
    layout->addRow(tr("Col span:"), colSpanSpin_);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

dashboard::PanelConfig AddPanelDialog::result() const
{
    dashboard::PanelConfig cfg;
    cfg.row = rowSpin_->value();
    cfg.col = colSpin_->value();
    cfg.rowSpan = rowSpanSpin_->value();
    cfg.colSpan = colSpanSpin_->value();
    cfg.title = titleEdit_->text();
    return cfg;
}

}  // namespace lumen::ui
