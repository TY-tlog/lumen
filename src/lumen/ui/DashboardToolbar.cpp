#include "DashboardToolbar.h"

#include <dashboard/Dashboard.h>

#include <QComboBox>
#include <QLabel>
#include <QPushButton>

namespace lumen::ui {

DashboardToolbar::DashboardToolbar(dashboard::Dashboard* dashboard,
                                     QWidget* parent)
    : QToolBar(tr("Dashboard"), parent)
    , dashboard_(dashboard)
{
    setMovable(false);
    setIconSize(QSize(16, 16));

    auto* addBtn = new QPushButton(QStringLiteral("+ Panel"), this);
    addBtn->setToolTip(tr("Add a new panel to the dashboard"));
    addWidget(addBtn);
    connect(addBtn, &QPushButton::clicked,
            this, &DashboardToolbar::addPanelRequested);

    addSeparator();

    addWidget(new QLabel(QStringLiteral(" Grid: "), this));
    gridCombo_ = new QComboBox(this);
    gridCombo_->addItem(QStringLiteral("1×1"), QVariant::fromValue(QPoint(1, 1)));
    gridCombo_->addItem(QStringLiteral("1×2"), QVariant::fromValue(QPoint(1, 2)));
    gridCombo_->addItem(QStringLiteral("2×1"), QVariant::fromValue(QPoint(2, 1)));
    gridCombo_->addItem(QStringLiteral("2×2"), QVariant::fromValue(QPoint(2, 2)));
    gridCombo_->addItem(QStringLiteral("2×3"), QVariant::fromValue(QPoint(2, 3)));
    gridCombo_->addItem(QStringLiteral("3×2"), QVariant::fromValue(QPoint(3, 2)));
    gridCombo_->addItem(QStringLiteral("3×3"), QVariant::fromValue(QPoint(3, 3)));
    gridCombo_->setCurrentIndex(3);
    addWidget(gridCombo_);
    connect(gridCombo_, &QComboBox::currentIndexChanged,
            this, &DashboardToolbar::onGridPresetChanged);

    addSeparator();

    auto* linkBtn = new QPushButton(QStringLiteral("Link Editor"), this);
    linkBtn->setToolTip(tr("Configure view linking between panels"));
    addWidget(linkBtn);
    connect(linkBtn, &QPushButton::clicked,
            this, &DashboardToolbar::linkEditorRequested);
}

void DashboardToolbar::onGridPresetChanged(int index)
{
    QPoint grid = gridCombo_->itemData(index).toPoint();
    if (grid.x() > 0 && grid.y() > 0) {
        dashboard_->setGridSize(grid.x(), grid.y());
    }
}

}  // namespace lumen::ui
