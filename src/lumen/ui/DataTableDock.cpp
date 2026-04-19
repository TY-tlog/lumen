#include "DataTableDock.h"

#include "DataFrameTableModel.h"

#include <data/Dataset.h>

#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStackedWidget>
#include <QTableView>

namespace lumen::ui {

DataTableDock::DataTableDock(QWidget* parent)
    : QDockWidget(tr("Data Table"), parent) {
    setObjectName(QStringLiteral("DataTableDock"));

    model_ = new DataFrameTableModel(this);

    proxyModel_ = new QSortFilterProxyModel(this);
    proxyModel_->setSourceModel(model_);

    tableView_ = new QTableView(this);
    tableView_->setModel(proxyModel_);
    tableView_->setSortingEnabled(true);
    tableView_->setAlternatingRowColors(true);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectItems);
    tableView_->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView_->horizontalHeader()->setStretchLastSection(true);
    tableView_->horizontalHeader()->setSectionsClickable(true);
    tableView_->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    tableView_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    tableView_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableView_, &QTableView::customContextMenuRequested,
            this, [this](const QPoint& pos) {
        QMenu menu(this);
        auto* resetSort = menu.addAction(tr("Reset sort order"));
        connect(resetSort, &QAction::triggered, this, [this]() {
            proxyModel_->sort(-1);  // Remove sort
            tableView_->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
        });
        menu.exec(tableView_->viewport()->mapToGlobal(pos));
    });

    placeholderLabel_ = new QLabel(this);
    placeholderLabel_->setAlignment(Qt::AlignCenter);
    placeholderLabel_->setWordWrap(true);

    stack_ = new QStackedWidget(this);
    stack_->addWidget(tableView_);       // index 0
    stack_->addWidget(placeholderLabel_); // index 1

    setWidget(stack_);
}

void DataTableDock::showDataFrame(const data::TabularBundle* bundle) {
    model_->setDataFrame(bundle);
    if (bundle != nullptr) {
        tableView_->resizeColumnsToContents();
    }
    stack_->setCurrentIndex(0); // show table
}

void DataTableDock::showDatasetInfo(const data::Dataset* ds) {
    if (ds == nullptr) {
        return;
    }

    const auto r = ds->rank();
    const auto s = ds->shape();

    if (r == 2 && s.size() >= 2) {
        placeholderLabel_->setText(
            QStringLiteral("Grid2D: %1 \u00d7 %2 (float64), ready for Phase 7 heatmap")
                .arg(s[0])
                .arg(s[1]));
        stack_->setCurrentIndex(1); // show placeholder
    } else if (r == 3 && s.size() >= 3) {
        placeholderLabel_->setText(
            QStringLiteral("Volume3D: %1 \u00d7 %2 \u00d7 %3 (float64), ready for Phase 8 rendering")
                .arg(s[0])
                .arg(s[1])
                .arg(s[2]));
        stack_->setCurrentIndex(1); // show placeholder
    } else {
        // For other ranks, show a generic message
        placeholderLabel_->setText(
            QStringLiteral("Dataset: rank %1, ready for visualization")
                .arg(r));
        stack_->setCurrentIndex(1);
    }
}

}  // namespace lumen::ui
