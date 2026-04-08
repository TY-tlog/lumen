#pragma once

#include <QString>

#include <cstddef>
#include <stdexcept>

namespace lumen::data {

/// Error thrown when CSV parsing fails.
///
/// Contains the line number, column number, and a human-readable description.
class CsvError : public std::runtime_error {
public:
    /// Construct an error with location and description.
    CsvError(std::size_t line, std::size_t column, const QString& description)
        : std::runtime_error(
            "CSV error at line " + std::to_string(line) + ", column "
            + std::to_string(column) + ": " + description.toStdString())
        , line_(line)
        , column_(column)
        , description_(description)
    {
    }

    /// Line number (1-based) where the error occurred.
    [[nodiscard]] std::size_t line() const { return line_; }

    /// Column number (1-based) where the error occurred.
    [[nodiscard]] std::size_t column() const { return column_; }

    /// Human-readable error description.
    [[nodiscard]] const QString& description() const { return description_; }

private:
    std::size_t line_;
    std::size_t column_;
    QString description_;
};

} // namespace lumen::data
