#include "DataFrame.h"

#include <stdexcept>
#include <string>

namespace lumen::data {

DataFrame::DataFrame(std::vector<Column> columns)
    : columns_(std::move(columns))
{
    if (columns_.size() > 1) {
        std::size_t expected = columns_[0].rowCount();
        for (std::size_t i = 1; i < columns_.size(); ++i) {
            if (columns_[i].rowCount() != expected) {
                throw std::invalid_argument(
                    "DataFrame: column " + std::to_string(i) + " has "
                    + std::to_string(columns_[i].rowCount()) + " rows, expected "
                    + std::to_string(expected));
            }
        }
    }
}

std::size_t DataFrame::rowCount() const
{
    if (columns_.empty()) {
        return 0;
    }
    return columns_[0].rowCount();
}

std::size_t DataFrame::columnCount() const
{
    return columns_.size();
}

const Column& DataFrame::column(std::size_t index) const
{
    if (index >= columns_.size()) {
        throw std::out_of_range("DataFrame::column: index " + std::to_string(index)
                                + " out of range (size=" + std::to_string(columns_.size())
                                + ")");
    }
    return columns_[index];
}

const Column* DataFrame::columnByName(const QString& name) const
{
    for (const auto& col : columns_) {
        if (col.name() == name) {
            return &col;
        }
    }
    return nullptr;
}

QVariant DataFrame::value(std::size_t row, std::size_t col) const
{
    if (col >= columns_.size()) {
        throw std::out_of_range("DataFrame::value: col index " + std::to_string(col)
                                + " out of range (size=" + std::to_string(columns_.size())
                                + ")");
    }
    return columns_[col].value(row);
}

} // namespace lumen::data
