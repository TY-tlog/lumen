#pragma once

#include "DataFrame.h"

#include <QString>

#include <cstddef>
#include <vector>

namespace lumen::data {

/// Configuration for the CSV reader.
struct CsvReaderOptions {
    /// Field delimiter character.
    char delimiter = ',';

    /// Maximum number of rows to scan for type inference (0 = scan all).
    std::size_t inferenceRows = 100;
};

/// Streaming RFC 4180 CSV parser.
///
/// Supports:
/// - Quoted fields with escaped quotes (double-quote)
/// - CRLF, LF, CR line endings
/// - UTF-8 BOM detection and skip
/// - Configurable delimiter
/// - Type inference (Int64, Double, String)
/// - Header detection
/// - NaN handling: "NaN", "nan", "NAN", "NA", empty string -> quiet_NaN for double columns
class CsvReader {
public:
    /// Construct a reader with the given options.
    explicit CsvReader(CsvReaderOptions options = {});

    /// Parse CSV from a file path. Throws CsvError on parse failure.
    [[nodiscard]] DataFrame readFile(const QString& filePath) const;

    /// Parse CSV from a string. Throws CsvError on parse failure.
    [[nodiscard]] DataFrame readString(const std::string& content) const;

private:
    /// Tokenize a single row from the input, advancing pos. Returns the fields.
    /// Sets lineNum to the starting line of the row (for error reporting).
    [[nodiscard]] std::vector<std::string> parseRow(const std::string& content, std::size_t& pos,
                                                    std::size_t& lineNum) const;

    /// Parse all rows from content.
    [[nodiscard]] std::vector<std::vector<std::string>> parseAllRows(
        const std::string& content) const;

    /// Check if a string looks numeric (int or double).
    [[nodiscard]] static bool isNumeric(const std::string& s);

    /// Check if a string represents a NaN/missing value.
    [[nodiscard]] static bool isNaN(const std::string& s);

    /// Detect whether row 0 is a header (all non-numeric strings).
    [[nodiscard]] static bool detectHeader(const std::vector<std::string>& firstRow);

    /// Infer column types from the first N rows.
    [[nodiscard]] std::vector<ColumnType> inferTypes(
        const std::vector<std::vector<std::string>>& rows) const;

    /// Build a DataFrame from parsed rows, header names, and inferred types.
    [[nodiscard]] DataFrame buildDataFrame(const std::vector<std::string>& headers,
                                           const std::vector<ColumnType>& types,
                                           const std::vector<std::vector<std::string>>& dataRows) const;

    /// Skip a UTF-8 BOM if present at the beginning of the content.
    [[nodiscard]] static std::size_t skipBom(const std::string& content);

    CsvReaderOptions options_;
};

} // namespace lumen::data
