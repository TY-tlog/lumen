#include "Column.h"

#include <cmath>
#include <stdexcept>

namespace lumen::data {

Column::Column(QString name, std::vector<int64_t> data)
    : name_(std::move(name))
    , type_(ColumnType::Int64)
    , storage_(std::move(data))
{
}

Column::Column(QString name, std::vector<double> data)
    : name_(std::move(name))
    , type_(ColumnType::Double)
    , storage_(std::move(data))
{
}

Column::Column(QString name, std::vector<QString> data)
    : name_(std::move(name))
    , type_(ColumnType::String)
    , storage_(std::move(data))
{
}

std::size_t Column::rowCount() const
{
    return std::visit([](const auto& vec) -> std::size_t { return vec.size(); }, storage_);
}

QVariant Column::value(std::size_t row) const
{
    if (row >= rowCount()) {
        throw std::out_of_range("Column::value: row index " + std::to_string(row)
                                + " out of range (size=" + std::to_string(rowCount()) + ")");
    }
    switch (type_) {
    case ColumnType::Int64:
        return QVariant::fromValue(std::get<std::vector<int64_t>>(storage_)[row]);
    case ColumnType::Double: {
        double v = std::get<std::vector<double>>(storage_)[row];
        if (std::isnan(v)) {
            return QVariant(QStringLiteral("NaN"));
        }
        return QVariant(v);
    }
    case ColumnType::String:
        return QVariant(std::get<std::vector<QString>>(storage_)[row]);
    }
    return {};
}

const std::vector<int64_t>& Column::int64Data() const
{
    return std::get<std::vector<int64_t>>(storage_);
}

const std::vector<double>& Column::doubleData() const
{
    return std::get<std::vector<double>>(storage_);
}

const std::vector<QString>& Column::stringData() const
{
    return std::get<std::vector<QString>>(storage_);
}

} // namespace lumen::data
