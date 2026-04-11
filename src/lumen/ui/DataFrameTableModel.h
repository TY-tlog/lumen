#pragma once

#include <QAbstractTableModel>

namespace lumen::data {
class TabularBundle;
}  // namespace lumen::data

namespace lumen::ui {

/// Read-only table model backed by a const TabularBundle pointer.
///
/// Formats doubles to 6 significant digits, displays NaN in text.tertiary
/// color, and aligns numbers right.
class DataFrameTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit DataFrameTableModel(QObject* parent = nullptr);

    /// Set the backing TabularBundle. Passing nullptr clears the model.
    void setDataFrame(const data::TabularBundle* bundle);

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section,
                                      Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;

private:
    const data::TabularBundle* bundle_ = nullptr;
};

}  // namespace lumen::ui
