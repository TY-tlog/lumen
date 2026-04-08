// Unit tests for DataFrame and Column.

#include <catch2/catch_test_macros.hpp>

#include <data/Column.h>
#include <data/ColumnType.h>
#include <data/DataFrame.h>

#include <QString>

#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

using namespace lumen::data;

// ===== Column tests =====

TEST_CASE("Column stores int64 data", "[column]") {
    std::vector<int64_t> data = {1, 2, 3, 4};
    Column col(QStringLiteral("test_int"), data);

    REQUIRE(col.name() == QStringLiteral("test_int"));
    REQUIRE(col.type() == ColumnType::Int64);
    REQUIRE(col.rowCount() == 4);
    REQUIRE(col.int64Data()[0] == 1);
    REQUIRE(col.int64Data()[3] == 4);
}

TEST_CASE("Column stores double data", "[column]") {
    std::vector<double> data = {1.5, 2.5, 3.5};
    Column col(QStringLiteral("test_dbl"), data);

    REQUIRE(col.type() == ColumnType::Double);
    REQUIRE(col.rowCount() == 3);
    REQUIRE(col.doubleData()[1] == 2.5);
}

TEST_CASE("Column stores QString data", "[column]") {
    std::vector<QString> data = {QStringLiteral("a"), QStringLiteral("b")};
    Column col(QStringLiteral("test_str"), data);

    REQUIRE(col.type() == ColumnType::String);
    REQUIRE(col.rowCount() == 2);
    REQUIRE(col.stringData()[0] == QStringLiteral("a"));
}

TEST_CASE("Column value() returns QVariant with bounds check", "[column]") {
    std::vector<int64_t> data = {10, 20};
    Column col(QStringLiteral("v"), data);

    REQUIRE(col.value(0).toLongLong() == 10);
    REQUIRE(col.value(1).toLongLong() == 20);
    REQUIRE_THROWS_AS(col.value(2), std::out_of_range);
}

TEST_CASE("Column value() returns NaN as string for double", "[column]") {
    std::vector<double> data = {1.0, std::numeric_limits<double>::quiet_NaN()};
    Column col(QStringLiteral("v"), data);

    REQUIRE(col.value(0).toDouble() == 1.0);
    REQUIRE(col.value(1).toString() == QStringLiteral("NaN"));
}

TEST_CASE("Column wrong type accessor throws", "[column]") {
    std::vector<int64_t> data = {1};
    Column col(QStringLiteral("v"), data);

    REQUIRE_THROWS_AS(col.doubleData(), std::bad_variant_access);
    REQUIRE_THROWS_AS(col.stringData(), std::bad_variant_access);
}

// ===== DataFrame tests =====

TEST_CASE("DataFrame default constructor is empty", "[dataframe]") {
    DataFrame df;
    REQUIRE(df.rowCount() == 0);
    REQUIRE(df.columnCount() == 0);
}

TEST_CASE("DataFrame from columns", "[dataframe]") {
    std::vector<Column> cols;
    cols.emplace_back(QStringLiteral("a"), std::vector<int64_t>{1, 2, 3});
    cols.emplace_back(QStringLiteral("b"), std::vector<double>{1.1, 2.2, 3.3});

    DataFrame df(std::move(cols));
    REQUIRE(df.rowCount() == 3);
    REQUIRE(df.columnCount() == 2);
}

TEST_CASE("DataFrame column access by index", "[dataframe]") {
    std::vector<Column> cols;
    cols.emplace_back(QStringLiteral("x"), std::vector<int64_t>{10});

    DataFrame df(std::move(cols));
    REQUIRE(df.column(0).name() == QStringLiteral("x"));
    REQUIRE_THROWS_AS(df.column(1), std::out_of_range);
}

TEST_CASE("DataFrame column lookup by name", "[dataframe]") {
    std::vector<Column> cols;
    cols.emplace_back(QStringLiteral("alpha"), std::vector<int64_t>{1});
    cols.emplace_back(QStringLiteral("beta"), std::vector<double>{2.0});

    DataFrame df(std::move(cols));
    REQUIRE(df.columnByName(QStringLiteral("alpha")) != nullptr);
    REQUIRE(df.columnByName(QStringLiteral("alpha"))->type() == ColumnType::Int64);
    REQUIRE(df.columnByName(QStringLiteral("beta")) != nullptr);
    REQUIRE(df.columnByName(QStringLiteral("gamma")) == nullptr);
}

TEST_CASE("DataFrame value() with bounds check", "[dataframe]") {
    std::vector<Column> cols;
    cols.emplace_back(QStringLiteral("c"), std::vector<int64_t>{5, 6});

    DataFrame df(std::move(cols));
    REQUIRE(df.value(0, 0).toLongLong() == 5);
    REQUIRE(df.value(1, 0).toLongLong() == 6);
    REQUIRE_THROWS_AS(df.value(0, 1), std::out_of_range);
    REQUIRE_THROWS_AS(df.value(2, 0), std::out_of_range);
}

TEST_CASE("DataFrame is move-only", "[dataframe]") {
    // Move construction
    std::vector<Column> cols;
    cols.emplace_back(QStringLiteral("m"), std::vector<int64_t>{1});

    DataFrame df1(std::move(cols));
    DataFrame df2(std::move(df1));
    REQUIRE(df2.columnCount() == 1);
    REQUIRE(df2.rowCount() == 1);

    // Copy is deleted (compile-time check; if this compiles, move-only is enforced)
    static_assert(!std::is_copy_constructible_v<DataFrame>);
    static_assert(!std::is_copy_assignable_v<DataFrame>);
}

TEST_CASE("DataFrame rejects mismatched row counts", "[dataframe]") {
    std::vector<Column> cols;
    cols.emplace_back(QStringLiteral("a"), std::vector<int64_t>{1, 2});
    cols.emplace_back(QStringLiteral("b"), std::vector<int64_t>{1});

    REQUIRE_THROWS_AS(DataFrame(std::move(cols)), std::invalid_argument);
}
