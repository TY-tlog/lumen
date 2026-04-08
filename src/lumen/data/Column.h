#pragma once

#include "ColumnType.h"

#include <QString>
#include <QVariant>

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace lumen::data {

/// Type-erased column storing one of int64_t, double, or QString vectors.
///
/// Provides name, type, row count, and typed accessors with bounds checking.
class Column {
public:
    /// Construct an Int64 column.
    Column(QString name, std::vector<int64_t> data);

    /// Construct a Double column.
    Column(QString name, std::vector<double> data);

    /// Construct a String column.
    Column(QString name, std::vector<QString> data);

    // Move-only
    Column(Column&&) noexcept = default;
    Column& operator=(Column&&) noexcept = default;
    Column(const Column&) = default;
    Column& operator=(const Column&) = default;

    /// Column name.
    [[nodiscard]] const QString& name() const { return name_; }

    /// Column data type.
    [[nodiscard]] ColumnType type() const { return type_; }

    /// Number of rows.
    [[nodiscard]] std::size_t rowCount() const;

    /// Access the value at row index as a QVariant. Throws std::out_of_range on bad index.
    [[nodiscard]] QVariant value(std::size_t row) const;

    /// Access int64 data. Throws std::bad_variant_access if wrong type.
    [[nodiscard]] const std::vector<int64_t>& int64Data() const;

    /// Access double data. Throws std::bad_variant_access if wrong type.
    [[nodiscard]] const std::vector<double>& doubleData() const;

    /// Access string data. Throws std::bad_variant_access if wrong type.
    [[nodiscard]] const std::vector<QString>& stringData() const;

private:
    QString name_;
    ColumnType type_;
    std::variant<std::vector<int64_t>, std::vector<double>, std::vector<QString>> storage_;
};

} // namespace lumen::data
