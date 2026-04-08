#include "DataTableDock.h"

#include "DataFrameTableModel.h"

#include <QHeaderView>
#include <QSortFilterProxyModel>
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

    setWidget(tableView_);
}

void DataTableDock::showDataFrame(const data::DataFrame* df) {
    model_->setDataFrame(df);
    if (df != nullptr) {
        tableView_->resizeColumnsToContents();
    }
}

}  // namespace lumen::ui
