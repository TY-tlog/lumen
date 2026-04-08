#include "CsvReader.h"
#include "CsvError.h"

#include <QDebug>
#include <QFile>
#include <QIODevice>

#include <algorithm>
#include <cerrno>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace lumen::data {

CsvReader::CsvReader(CsvReaderOptions options)
    : options_(std::move(options))
{
}

std::size_t CsvReader::skipBom(const std::string& content)
{
    // UTF-8 BOM: EF BB BF
    if (content.size() >= 3
        && static_cast<unsigned char>(content[0]) == 0xEF
        && static_cast<unsigned char>(content[1]) == 0xBB
        && static_cast<unsigned char>(content[2]) == 0xBF) {
        return 3;
    }
    return 0;
}

std::vector<std::string> CsvReader::parseRow(const std::string& content, std::size_t& pos,
                                              std::size_t& lineNum) const
{
    std::vector<std::string> fields;
    std::string field;
    const char delim = options_.delimiter;

    bool inQuotes = false;
    bool fieldStarted = false;

    while (pos < content.size()) {
        char ch = content[pos];

        if (inQuotes) {
            if (ch == '"') {
                // Check for escaped quote ""
                if (pos + 1 < content.size() && content[pos + 1] == '"') {
                    field += '"';
                    pos += 2;
                } else {
                    // End of quoted field
                    inQuotes = false;
                    ++pos;
                }
            } else {
                if (ch == '\n') {
                    ++lineNum;
                } else if (ch == '\r') {
                    ++lineNum;
                    if (pos + 1 < content.size() && content[pos + 1] == '\n') {
                        field += '\n';
                        pos += 2;
                        continue;
                    }
                }
                field += ch;
                ++pos;
            }
        } else {
            if (ch == '"' && !fieldStarted) {
                inQuotes = true;
                fieldStarted = true;
                ++pos;
            } else if (ch == delim) {
                fields.push_back(std::move(field));
                field.clear();
                fieldStarted = false;
                ++pos;
            } else if (ch == '\r') {
                // CR or CRLF
                ++pos;
                if (pos < content.size() && content[pos] == '\n') {
                    ++pos;
                }
                ++lineNum;
                fields.push_back(std::move(field));
                return fields;
            } else if (ch == '\n') {
                ++pos;
                ++lineNum;
                fields.push_back(std::move(field));
                return fields;
            } else {
                field += ch;
                fieldStarted = true;
                ++pos;
            }
        }
    }

    // End of input — push the last field if we have any content or fields already accumulated
    if (!field.empty() || !fields.empty() || fieldStarted) {
        fields.push_back(std::move(field));
    }

    if (inQuotes) {
        throw CsvError(lineNum, fields.size() + 1,
                        QStringLiteral("Unterminated quoted field at end of input"));
    }

    return fields;
}

std::vector<std::vector<std::string>> CsvReader::parseAllRows(const std::string& content) const
{
    std::vector<std::vector<std::string>> rows;
    std::size_t pos = skipBom(content);
    std::size_t lineNum = 1;

    while (pos < content.size()) {
        auto row = parseRow(content, pos, lineNum);
        if (!row.empty()) {
            rows.push_back(std::move(row));
        }
    }

    return rows;
}

bool CsvReader::isNaN(const std::string& s)
{
    return s.empty() || s == "NaN" || s == "nan" || s == "NAN" || s == "NA";
}

bool CsvReader::isNumeric(const std::string& s)
{
    if (s.empty()) {
        return false;
    }

    // Try int64 first
    {
        int64_t val{};
        auto result = std::from_chars(s.data(), s.data() + s.size(), val);
        if (result.ec == std::errc{} && result.ptr == s.data() + s.size()) {
            return true;
        }
    }

    // Try double
    {
        double val{};
        auto result = std::from_chars(s.data(), s.data() + s.size(), val);
        if (result.ec == std::errc{} && result.ptr == s.data() + s.size()) {
            return true;
        }
    }

    return false;
}

bool CsvReader::detectHeader(const std::vector<std::string>& firstRow)
{
    return std::all_of(firstRow.begin(), firstRow.end(), [](const std::string& s) {
        return !s.empty() && !isNumeric(s) && !isNaN(s);
    });
}

std::vector<ColumnType> CsvReader::inferTypes(
    const std::vector<std::vector<std::string>>& rows) const
{
    if (rows.empty()) {
        return {};
    }

    std::size_t numCols = rows[0].size();
    std::vector<ColumnType> types(numCols, ColumnType::Int64);

    // Track whether we've seen any non-NaN value per column
    std::vector<bool> hasNonNaN(numCols, false);

    std::size_t scanRows = rows.size();
    if (options_.inferenceRows > 0 && scanRows > options_.inferenceRows) {
        scanRows = options_.inferenceRows;
    }

    for (std::size_t r = 0; r < scanRows; ++r) {
        for (std::size_t c = 0; c < numCols && c < rows[r].size(); ++c) {
            const std::string& val = rows[r][c];

            if (isNaN(val)) {
                continue; // NaN is compatible with any numeric type
            }

            hasNonNaN[c] = true;

            if (types[c] == ColumnType::String) {
                continue; // Already widened to String
            }

            // Try int64
            if (types[c] == ColumnType::Int64) {
                int64_t ival{};
                auto result = std::from_chars(val.data(), val.data() + val.size(), ival);
                if (result.ec == std::errc{} && result.ptr == val.data() + val.size()) {
                    continue; // Still Int64
                }
                // Can't be int, try double
                types[c] = ColumnType::Double;
            }

            if (types[c] == ColumnType::Double) {
                double dval{};
                auto result = std::from_chars(val.data(), val.data() + val.size(), dval);
                if (result.ec == std::errc{} && result.ptr == val.data() + val.size()) {
                    continue; // Still Double
                }
                // Not numeric
                types[c] = ColumnType::String;
            }
        }
    }

    // All-NaN columns default to Double (per spec)
    for (std::size_t c = 0; c < numCols; ++c) {
        if (!hasNonNaN[c]) {
            types[c] = ColumnType::Double;
        }
    }

    return types;
}

DataFrame CsvReader::buildDataFrame(const std::vector<std::string>& headers,
                                     const std::vector<ColumnType>& types,
                                     const std::vector<std::vector<std::string>>& dataRows) const
{
    std::size_t numCols = headers.size();
    std::size_t numRows = dataRows.size();

    // Prepare storage per column
    std::vector<std::vector<int64_t>> intCols(numCols);
    std::vector<std::vector<double>> dblCols(numCols);
    std::vector<std::vector<QString>> strCols(numCols);

    for (std::size_t c = 0; c < numCols; ++c) {
        switch (types[c]) {
        case ColumnType::Int64:
            intCols[c].reserve(numRows);
            break;
        case ColumnType::Double:
            dblCols[c].reserve(numRows);
            break;
        case ColumnType::String:
            strCols[c].reserve(numRows);
            break;
        }
    }

    for (std::size_t r = 0; r < numRows; ++r) {
        const auto& row = dataRows[r];
        for (std::size_t c = 0; c < numCols; ++c) {
            const std::string& val = (c < row.size()) ? row[c] : std::string{};

            switch (types[c]) {
            case ColumnType::Int64: {
                if (isNaN(val)) {
                    // Int64 column with NaN — shouldn't happen if inference is correct,
                    // but store 0 as fallback
                    intCols[c].push_back(0);
                } else {
                    int64_t ival{};
                    std::from_chars(val.data(), val.data() + val.size(), ival);
                    intCols[c].push_back(ival);
                }
                break;
            }
            case ColumnType::Double: {
                if (isNaN(val)) {
                    dblCols[c].push_back(std::numeric_limits<double>::quiet_NaN());
                } else {
                    double dval{};
                    std::from_chars(val.data(), val.data() + val.size(), dval);
                    dblCols[c].push_back(dval);
                }
                break;
            }
            case ColumnType::String: {
                strCols[c].push_back(QString::fromStdString(val));
                break;
            }
            }
        }
    }

    // Build Column objects
    std::vector<Column> columns;
    columns.reserve(numCols);
    for (std::size_t c = 0; c < numCols; ++c) {
        QString colName = QString::fromStdString(headers[c]);
        switch (types[c]) {
        case ColumnType::Int64:
            columns.emplace_back(colName, std::move(intCols[c]));
            break;
        case ColumnType::Double:
            columns.emplace_back(colName, std::move(dblCols[c]));
            break;
        case ColumnType::String:
            columns.emplace_back(colName, std::move(strCols[c]));
            break;
        }
    }

    return DataFrame(std::move(columns));
}

DataFrame CsvReader::readFile(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        throw CsvError(0, 0,
                        QStringLiteral("Cannot open file: ") + filePath + QStringLiteral(" — ")
                            + file.errorString());
    }

    QByteArray bytes = file.readAll();
    std::string content(bytes.constData(), static_cast<std::size_t>(bytes.size()));
    return readString(content);
}

DataFrame CsvReader::readString(const std::string& content) const
{
    if (content.empty()) {
        return DataFrame();
    }

    // Check if content is only BOM
    std::size_t bomLen = skipBom(content);
    if (bomLen >= content.size()) {
        return DataFrame();
    }

    auto rows = parseAllRows(content);
    if (rows.empty()) {
        return DataFrame();
    }

    // Validate: check consistent column count
    std::size_t numCols = rows[0].size();

    // Detect header
    bool hasHeader = detectHeader(rows[0]);

    // Build headers
    std::vector<std::string> headers;
    headers.reserve(numCols);
    std::size_t dataStart = 0;

    if (hasHeader) {
        for (const auto& h : rows[0]) {
            headers.push_back(h);
        }
        dataStart = 1;
    } else {
        for (std::size_t i = 0; i < numCols; ++i) {
            headers.push_back("col_" + std::to_string(i));
        }
    }

    // Data rows
    std::vector<std::vector<std::string>> dataRows(rows.begin() + static_cast<ptrdiff_t>(dataStart),
                                                    rows.end());

    // Validate column counts in data rows
    for (std::size_t r = 0; r < dataRows.size(); ++r) {
        if (dataRows[r].size() != numCols) {
            std::size_t sourceLine = r + dataStart + 1; // 1-based
            throw CsvError(
                sourceLine, dataRows[r].size(),
                QStringLiteral("Expected ") + QString::number(static_cast<qulonglong>(numCols))
                    + QStringLiteral(" columns but got ")
                    + QString::number(static_cast<qulonglong>(dataRows[r].size())));
        }
    }

    // Infer types from data rows
    auto types = inferTypes(dataRows);

    // Build and return the DataFrame
    return buildDataFrame(headers, types, dataRows);
}

} // namespace lumen::data
