// Unit tests for CsvReader.

#include <catch2/catch_test_macros.hpp>

#include <data/CsvError.h>
#include <data/CsvReader.h>
#include <data/DataFrame.h>

#include <QCoreApplication>
#include <QString>

#include <cmath>
#include <limits>
#include <string>

using namespace lumen::data;

// Helper: path to test fixtures
static QString fixturePath(const char* name)
{
    return QStringLiteral(LUMEN_TEST_FIXTURES_DIR) + QStringLiteral("/tiny/") + QString::fromUtf8(name);
}

// ===== Basic parsing =====

TEST_CASE("Parse simple 3x4 CSV", "[csv]") {
    CsvReader reader;
    auto df = reader.readFile(fixturePath("simple_3x4.csv"));

    REQUIRE(df.columnCount() == 3);
    REQUIRE(df.rowCount() == 4);

    // Headers
    REQUIRE(df.column(0).name() == QStringLiteral("a"));
    REQUIRE(df.column(1).name() == QStringLiteral("b"));
    REQUIRE(df.column(2).name() == QStringLiteral("c"));

    // Types: all integer
    REQUIRE(df.column(0).type() == ColumnType::Int64);
    REQUIRE(df.column(1).type() == ColumnType::Int64);

    // Values
    REQUIRE(df.column(0).int64Data()[0] == 1);
    REQUIRE(df.column(0).int64Data()[3] == 10);
    REQUIRE(df.column(2).int64Data()[2] == 9);
}

TEST_CASE("Parse empty CSV returns empty DataFrame", "[csv]") {
    CsvReader reader;
    auto df = reader.readFile(fixturePath("empty.csv"));

    REQUIRE(df.columnCount() == 0);
    REQUIRE(df.rowCount() == 0);
}

TEST_CASE("Parse single column CSV", "[csv]") {
    CsvReader reader;
    auto df = reader.readFile(fixturePath("single_column.csv"));

    REQUIRE(df.columnCount() == 1);
    REQUIRE(df.rowCount() == 3);
    REQUIRE(df.column(0).name() == QStringLiteral("value"));
    REQUIRE(df.column(0).type() == ColumnType::Double);
    REQUIRE(df.column(0).doubleData()[0] == 1.5);
}

// ===== NaN handling =====

TEST_CASE("Parse CSV with NaN values", "[csv]") {
    CsvReader reader;
    auto df = reader.readFile(fixturePath("with_nan.csv"));

    REQUIRE(df.columnCount() == 3);
    REQUIRE(df.rowCount() == 5);

    // All columns should be Double (because of NaN mixing)
    REQUIRE(df.column(0).type() == ColumnType::Double);
    REQUIRE(df.column(1).type() == ColumnType::Double);
    REQUIRE(df.column(2).type() == ColumnType::Double);

    // Row 0: 1.0, 2.0, 3.0
    REQUIRE(df.column(0).doubleData()[0] == 1.0);

    // Row 1: NaN, 4.0, 5.0
    REQUIRE(std::isnan(df.column(0).doubleData()[1]));
    REQUIRE(df.column(1).doubleData()[1] == 4.0);

    // Row 2: 6.0, nan, 7.0
    REQUIRE(std::isnan(df.column(1).doubleData()[2]));

    // Row 3: 8.0, 9.0, NAN
    REQUIRE(std::isnan(df.column(2).doubleData()[3]));

    // Row 4: 10.0, NA, (empty)
    REQUIRE(std::isnan(df.column(1).doubleData()[4]));
    REQUIRE(std::isnan(df.column(2).doubleData()[4]));
}

TEST_CASE("NaN string variants all become quiet_NaN", "[csv]") {
    CsvReader reader;
    std::string csv = "h\nNaN\nnan\nNAN\nNA\n\n";
    auto df = reader.readString(csv);

    REQUIRE(df.columnCount() == 1);
    REQUIRE(df.column(0).type() == ColumnType::Double);
    for (std::size_t i = 0; i < df.rowCount(); ++i) {
        REQUIRE(std::isnan(df.column(0).doubleData()[i]));
    }
}

// ===== Header detection =====

TEST_CASE("No header — numeric first row generates col_N names", "[csv]") {
    CsvReader reader;
    auto df = reader.readFile(fixturePath("no_header.csv"));

    REQUIRE(df.columnCount() == 3);
    REQUIRE(df.rowCount() == 3);
    REQUIRE(df.column(0).name() == QStringLiteral("col_0"));
    REQUIRE(df.column(1).name() == QStringLiteral("col_1"));
    REQUIRE(df.column(2).name() == QStringLiteral("col_2"));
}

// ===== RFC 4180 quoting =====

TEST_CASE("RFC 4180 quoted fields", "[csv]") {
    CsvReader reader;
    auto df = reader.readFile(fixturePath("rfc4180_quoted.csv"));

    REQUIRE(df.columnCount() == 3);
    REQUIRE(df.rowCount() == 4);

    // All columns should be String (mixed content)
    REQUIRE(df.column(0).type() == ColumnType::String);

    // Row 0: simple, 1, no quotes
    REQUIRE(df.column(0).stringData()[0] == QStringLiteral("simple"));

    // Row 1: with "quotes", 2, has "escaped" quotes
    REQUIRE(df.column(0).stringData()[1] == QStringLiteral("with \"quotes\""));
    REQUIRE(df.column(2).stringData()[1] == QStringLiteral("has \"escaped\" quotes"));

    // Row 2: with,comma, 3, comma inside
    REQUIRE(df.column(0).stringData()[2] == QStringLiteral("with,comma"));

    // Row 3: with\nnewline, 4, newline inside
    REQUIRE(df.column(0).stringData()[3].contains(QStringLiteral("\n")));
}

// ===== UTF-8 BOM =====

TEST_CASE("UTF-8 BOM is skipped", "[csv]") {
    CsvReader reader;
    auto df = reader.readFile(fixturePath("with_bom.csv"));

    REQUIRE(df.columnCount() == 3);
    REQUIRE(df.column(0).name() == QStringLiteral("a"));
    REQUIRE(df.rowCount() == 2);
}

// ===== Inline string parsing =====

TEST_CASE("Parse from string", "[csv]") {
    CsvReader reader;
    std::string csv = "x,y\n1,2\n3,4\n";
    auto df = reader.readString(csv);

    REQUIRE(df.columnCount() == 2);
    REQUIRE(df.rowCount() == 2);
    REQUIRE(df.column(0).name() == QStringLiteral("x"));
    REQUIRE(df.column(0).int64Data()[0] == 1);
}

TEST_CASE("Parse empty string returns empty DataFrame", "[csv]") {
    CsvReader reader;
    auto df = reader.readString("");

    REQUIRE(df.columnCount() == 0);
    REQUIRE(df.rowCount() == 0);
}

// ===== CRLF handling =====

TEST_CASE("CRLF line endings", "[csv]") {
    CsvReader reader;
    std::string csv = "a,b\r\n1,2\r\n3,4\r\n";
    auto df = reader.readString(csv);

    REQUIRE(df.columnCount() == 2);
    REQUIRE(df.rowCount() == 2);
    REQUIRE(df.column(0).int64Data()[0] == 1);
    REQUIRE(df.column(1).int64Data()[1] == 4);
}

TEST_CASE("CR-only line endings", "[csv]") {
    CsvReader reader;
    std::string csv = "a,b\r1,2\r3,4\r";
    auto df = reader.readString(csv);

    REQUIRE(df.columnCount() == 2);
    REQUIRE(df.rowCount() == 2);
}

// ===== Custom delimiter =====

TEST_CASE("Tab delimiter", "[csv]") {
    CsvReader reader(CsvReaderOptions{'\t'});
    std::string csv = "a\tb\n1\t2\n3\t4\n";
    auto df = reader.readString(csv);

    REQUIRE(df.columnCount() == 2);
    REQUIRE(df.rowCount() == 2);
}

// ===== Type inference =====

TEST_CASE("Mixed int and double column infers Double", "[csv]") {
    CsvReader reader;
    std::string csv = "val\n1\n2.5\n3\n";
    auto df = reader.readString(csv);

    REQUIRE(df.column(0).type() == ColumnType::Double);
    REQUIRE(df.column(0).doubleData()[0] == 1.0);
    REQUIRE(df.column(0).doubleData()[1] == 2.5);
}

TEST_CASE("All-NaN column infers as Double", "[csv]") {
    CsvReader reader;
    std::string csv = "x,y\n1,NaN\n2,\n3,NA\n";
    auto df = reader.readString(csv);

    REQUIRE(df.column(1).type() == ColumnType::Double);
    for (std::size_t i = 0; i < 3; ++i) {
        REQUIRE(std::isnan(df.column(1).doubleData()[i]));
    }
}

TEST_CASE("String column detected when non-numeric values present", "[csv]") {
    CsvReader reader;
    std::string csv = "label,val\nhello,1\nworld,2\n";
    auto df = reader.readString(csv);

    REQUIRE(df.column(0).type() == ColumnType::String);
    REQUIRE(df.column(0).stringData()[0] == QStringLiteral("hello"));
    REQUIRE(df.column(1).type() == ColumnType::Int64);
}

// ===== Error handling =====

TEST_CASE("Mismatched column count throws CsvError", "[csv]") {
    CsvReader reader;
    std::string csv = "a,b,c\n1,2,3\n4,5\n";

    REQUIRE_THROWS_AS(reader.readString(csv), CsvError);
}

TEST_CASE("Unterminated quoted field throws CsvError", "[csv]") {
    CsvReader reader;
    std::string csv = "a,b\n\"unterminated,1\n";

    REQUIRE_THROWS_AS(reader.readString(csv), CsvError);
}

TEST_CASE("CsvError has line and column info", "[csv]") {
    CsvReader reader;
    std::string csv = "a,b,c\n1,2,3\n4,5\n";

    try {
        auto df = reader.readString(csv);
        static_cast<void>(df);
        REQUIRE(false); // Should not reach here
    } catch (const CsvError& e) {
        REQUIRE(e.line() == 3);
        REQUIRE(!e.description().isEmpty());
    }
}

TEST_CASE("File not found throws CsvError", "[csv]") {
    CsvReader reader;
    REQUIRE_THROWS_AS(reader.readFile(QStringLiteral("/nonexistent/path.csv")), CsvError);
}
