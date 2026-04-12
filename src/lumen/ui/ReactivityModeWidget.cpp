#include "ReactivityModeWidget.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QRadioButton>

namespace lumen::ui {

ReactivityModeWidget::ReactivityModeWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    group_ = new QButtonGroup(this);

    staticBtn_ = new QRadioButton(tr("Static"), this);
    staticBtn_->setToolTip(
        tr("Plot shows a frozen snapshot. Changes to data don't update the plot."));

    dagBtn_ = new QRadioButton(tr("DAG"), this);
    dagBtn_->setToolTip(
        tr("Plot updates automatically when data changes. One-way."));

    bidiBtn_ = new QRadioButton(tr("Bidirectional"), this);
    bidiBtn_->setToolTip(
        tr("Plot and data are linked. Edits in the plot write back to data."));

    group_->addButton(staticBtn_, static_cast<int>(reactive::Mode::Static));
    group_->addButton(dagBtn_, static_cast<int>(reactive::Mode::DAG));
    group_->addButton(bidiBtn_, static_cast<int>(reactive::Mode::Bidirectional));

    layout->addWidget(staticBtn_);
    layout->addWidget(dagBtn_);
    layout->addWidget(bidiBtn_);

    // Default to DAG.
    dagBtn_->setChecked(true);

    connect(group_, &QButtonGroup::idToggled,
            this, &ReactivityModeWidget::onButtonToggled);
}

void ReactivityModeWidget::setMode(reactive::Mode m)
{
    // Block signals so programmatic changes don't emit modeChanged.
    group_->blockSignals(true);
    switch (m) {
    case reactive::Mode::Static:
        staticBtn_->setChecked(true);
        break;
    case reactive::Mode::DAG:
        dagBtn_->setChecked(true);
        break;
    case reactive::Mode::Bidirectional:
        bidiBtn_->setChecked(true);
        break;
    }
    group_->blockSignals(false);
}

reactive::Mode ReactivityModeWidget::mode() const
{
    int id = group_->checkedId();
    return static_cast<reactive::Mode>(id);
}

void ReactivityModeWidget::onButtonToggled(int id, bool checked)
{
    if (checked) {
        emit modeChanged(static_cast<reactive::Mode>(id));
    }
}

}  // namespace lumen::ui
