#include "DataFrameTableModel.h"

#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>
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

void DataFrameTableModel::setDataFrame(const data::TabularBundle* bundle) {
    beginResetModel();
    bundle_ = bundle;
    endResetModel();
}

int DataFrameTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid() || bundle_ == nullptr) {
        return 0;
    }
    return static_cast<int>(bundle_->rowCount());
}

int DataFrameTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid() || bundle_ == nullptr) {
        return 0;
    }
    return bundle_->columnCount();
}

QVariant DataFrameTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || bundle_ == nullptr) {
        return {};
    }

    const auto row = static_cast<std::size_t>(index.row());
    const auto col = index.column();

    if (row >= bundle_->rowCount() || col >= bundle_->columnCount()) {
        return {};
    }

    auto ds = bundle_->column(col);
    if (!ds) {
        return {};
    }

    // Determine data type by trying accessors
    // Try double first
    bool isDouble = false;
    bool isInt64 = false;
    try {
        (void)ds->doubleData();
        isDouble = true;
    } catch (...) {}

    if (!isDouble) {
        try {
            (void)ds->int64Data();
            isInt64 = true;
        } catch (...) {}
    }

    if (role == Qt::DisplayRole) {
        if (isDouble) {
            double val = ds->doubleData()[row];
            if (std::isnan(val)) {
                return QStringLiteral("NaN");
            }
            return QString::number(val, 'g', 6);
        }
        if (isInt64) {
            return QString::number(ds->int64Data()[row]);
        }
        // String
        try {
            return ds->stringData()[row];
        } catch (...) {
            return {};
        }
    }

    if (role == Qt::ForegroundRole) {
        if (isDouble) {
            double val = ds->doubleData()[row];
            if (std::isnan(val)) {
                return QBrush(tokens::color::text::tertiary);
            }
        }
        return {};
    }

    if (role == Qt::TextAlignmentRole) {
        if (isDouble || isInt64) {
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        }
        return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    }

    return {};
}

QVariant DataFrameTableModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
    if (role != Qt::DisplayRole || bundle_ == nullptr) {
        return {};
    }

    if (orientation == Qt::Horizontal) {
        if (section >= 0 && section < bundle_->columnCount()) {
            auto ds = bundle_->column(section);
            if (ds) {
                return ds->name();
            }
        }
    } else {
        // Row numbers (1-based)
        return section + 1;
    }

    return {};
}

}  // namespace lumen::ui
