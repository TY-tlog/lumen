#include "DataFrameTableModel.h"

#include <data/Column.h>
#include <data/ColumnType.h>
#include <data/DataFrame.h>
#include <style/DesignTokens.h>

#include <QBrush>
#include <QString>

#include <cmath>
#include <cstdint>
#include <limits>

namespace lumen::ui {

DataFrameTableModel::DataFrameTableModel(QObject* parent)
    : QAbstractTableModel(parent) {
}

void DataFrameTableModel::setDataFrame(const data::DataFrame* df) {
    beginResetModel();
    dataFrame_ = df;
    endResetModel();
}

int DataFrameTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid() || dataFrame_ == nullptr) {
        return 0;
    }
    return static_cast<int>(dataFrame_->rowCount());
}

int DataFrameTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid() || dataFrame_ == nullptr) {
        return 0;
    }
    return static_cast<int>(dataFrame_->columnCount());
}

QVariant DataFrameTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || dataFrame_ == nullptr) {
        return {};
    }

    const auto row = static_cast<std::size_t>(index.row());
    const auto col = static_cast<std::size_t>(index.column());

    if (row >= dataFrame_->rowCount() || col >= dataFrame_->columnCount()) {
        return {};
    }

    const auto& column = dataFrame_->column(col);

    if (role == Qt::DisplayRole) {
        switch (column.type()) {
        case data::ColumnType::Double: {
            double val = column.doubleData()[row];
            if (std::isnan(val)) {
                return QStringLiteral("NaN");
            }
            return QString::number(val, 'g', 6);
        }
        case data::ColumnType::Int64:
            return QString::number(column.int64Data()[row]);
        case data::ColumnType::String:
            return column.stringData()[row];
        }
        return {};
    }

    if (role == Qt::ForegroundRole) {
        if (column.type() == data::ColumnType::Double) {
            double val = column.doubleData()[row];
            if (std::isnan(val)) {
                return QBrush(tokens::color::text::tertiary);
            }
        }
        return {};
    }

    if (role == Qt::TextAlignmentRole) {
        switch (column.type()) {
        case data::ColumnType::Double:
        case data::ColumnType::Int64:
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        case data::ColumnType::String:
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
        return {};
    }

    return {};
}

QVariant DataFrameTableModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
    if (role != Qt::DisplayRole || dataFrame_ == nullptr) {
        return {};
    }

    if (orientation == Qt::Horizontal) {
        if (section >= 0 && static_cast<std::size_t>(section) < dataFrame_->columnCount()) {
            return dataFrame_->column(static_cast<std::size_t>(section)).name();
        }
    } else {
        // Row numbers (1-based)
        return section + 1;
    }

    return {};
}

}  // namespace lumen::ui
