#pragma once

#include <QDockWidget>

class QSortFilterProxyModel;
class QTableView;

namespace lumen::data {
class DataFrame;
}  // namespace lumen::data

namespace lumen::ui {

class DataFrameTableModel;

/// Dock widget containing a sortable table view of a DataFrame.
class DataTableDock : public QDockWidget {
    Q_OBJECT

public:
    explicit DataTableDock(QWidget* parent = nullptr);

    /// Display the given DataFrame in the table. Passing nullptr clears it.
    void showDataFrame(const data::DataFrame* df);

private:
    DataFrameTableModel* model_ = nullptr;
    QSortFilterProxyModel* proxyModel_ = nullptr;
    QTableView* tableView_ = nullptr;
};

}  // namespace lumen::ui
