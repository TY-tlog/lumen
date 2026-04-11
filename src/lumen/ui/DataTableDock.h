#pragma once

#include <QDockWidget>

class QSortFilterProxyModel;
class QTableView;

namespace lumen::data {
class TabularBundle;
}  // namespace lumen::data

namespace lumen::ui {

class DataFrameTableModel;

/// Dock widget containing a sortable table view of a TabularBundle.
class DataTableDock : public QDockWidget {
    Q_OBJECT

public:
    explicit DataTableDock(QWidget* parent = nullptr);

    /// Display the given TabularBundle in the table. Passing nullptr clears it.
    void showDataFrame(const data::TabularBundle* bundle);

private:
    DataFrameTableModel* model_ = nullptr;
    QSortFilterProxyModel* proxyModel_ = nullptr;
    QTableView* tableView_ = nullptr;
};

}  // namespace lumen::ui
