#pragma once

#include <QDockWidget>

class QLabel;
class QSortFilterProxyModel;
class QStackedWidget;
class QTableView;

namespace lumen::data {
class Dataset;
class TabularBundle;
}  // namespace lumen::data

namespace lumen::ui {

class DataFrameTableModel;

/// Dock widget containing a sortable table view of a TabularBundle,
/// or a placeholder label for Grid2D / Volume3D datasets.
class DataTableDock : public QDockWidget {
    Q_OBJECT

public:
    explicit DataTableDock(QWidget* parent = nullptr);

    /// Display the given TabularBundle in the table. Passing nullptr clears it.
    void showDataFrame(const data::TabularBundle* bundle);

    /// Display info about a Dataset (Grid2D / Volume3D placeholder, or tabular).
    void showDatasetInfo(const data::Dataset* ds);

private:
    DataFrameTableModel* model_ = nullptr;
    QSortFilterProxyModel* proxyModel_ = nullptr;
    QTableView* tableView_ = nullptr;
    QLabel* placeholderLabel_ = nullptr;
    QStackedWidget* stack_ = nullptr;
};

}  // namespace lumen::ui
