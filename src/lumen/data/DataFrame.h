#pragma once

#include "Column.h"

#include <QVariant>

#include <cstddef>
#include <optional>
#include <vector>

namespace lumen::data {

/// Tabular data container owning a set of named, typed columns.
///
/// Move-only. Provides column-by-name lookup, row/col counts,
/// and bounds-checked value access.
class DataFrame {
public:
    DataFrame() = default;

    /// Construct from a vector of columns. All columns must have the same row count.
    explicit DataFrame(std::vector<Column> columns);

    // Move-only
    DataFrame(DataFrame&&) noexcept = default;
    DataFrame& operator=(DataFrame&&) noexcept = default;
    DataFrame(const DataFrame&) = delete;
    DataFrame& operator=(const DataFrame&) = delete;

    /// Number of rows (0 if no columns).
    [[nodiscard]] std::size_t rowCount() const;

    /// Number of columns.
    [[nodiscard]] std::size_t columnCount() const;

    /// Access column by index. Throws std::out_of_range if out of bounds.
    [[nodiscard]] const Column& column(std::size_t index) const;

    /// Access column by name. Returns nullptr if not found.
    [[nodiscard]] const Column* columnByName(const QString& name) const;

    /// Access a single cell value. Throws std::out_of_range if out of bounds.
    [[nodiscard]] QVariant value(std::size_t row, std::size_t col) const;

    /// Access the underlying columns vector.
    [[nodiscard]] const std::vector<Column>& columns() const { return columns_; }

private:
    std::vector<Column> columns_;
};

} // namespace lumen::data
