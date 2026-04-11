// Unit tests for TabularBundle and Rank1Dataset (migrated from DataFrame/Column tests).

#include <catch2/catch_test_macros.hpp>

#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>
#include <data/Unit.h>

#include <QString>

#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

using namespace lumen::data;

// ===== Rank1Dataset tests (migrated from Column) =====

TEST_CASE("Rank1Dataset stores int64 data", "[column]") {
    auto col = std::make_shared<Rank1Dataset>(QStringLiteral("test_int"), Unit::dimensionless(),
                                               std::vector<int64_t>{1, 2, 3, 4});
    REQUIRE(col->name() == QStringLiteral("test_int"));
    REQUIRE(col->rowCount() == 4);
    REQUIRE(col->int64Data()[0] == 1);
    REQUIRE(col->int64Data()[3] == 4);
}

TEST_CASE("Rank1Dataset stores double data", "[column]") {
    auto col = std::make_shared<Rank1Dataset>(QStringLiteral("test_dbl"), Unit::dimensionless(),
                                               std::vector<double>{1.5, 2.5, 3.5});
    REQUIRE(col->rowCount() == 3);
    REQUIRE(col->doubleData()[1] == 2.5);
}

TEST_CASE("Rank1Dataset stores QString data", "[column]") {
    auto col = std::make_shared<Rank1Dataset>(QStringLiteral("test_str"), Unit::dimensionless(),
                                               std::vector<QString>{QStringLiteral("a"), QStringLiteral("b")});
    REQUIRE(col->rowCount() == 2);
    REQUIRE(col->stringData()[0] == QStringLiteral("a"));
}

TEST_CASE("Rank1Dataset valueAt returns double", "[column]") {
    auto col = std::make_shared<Rank1Dataset>(QStringLiteral("v"), Unit::dimensionless(),
                                               std::vector<int64_t>{10, 20});
    REQUIRE(col->valueAt({0}) == 10.0);
    REQUIRE(col->valueAt({1}) == 20.0);
    REQUIRE_THROWS_AS(col->valueAt({2}), std::out_of_range);
}

TEST_CASE("Rank1Dataset wrong type accessor throws", "[column]") {
    auto col = std::make_shared<Rank1Dataset>(QStringLiteral("v"), Unit::dimensionless(),
                                               std::vector<int64_t>{1});
    REQUIRE_THROWS_AS(col->doubleData(), std::bad_variant_access);
    REQUIRE_THROWS_AS(col->stringData(), std::bad_variant_access);
}

// ===== TabularBundle tests (migrated from DataFrame) =====

TEST_CASE("TabularBundle default constructor is empty", "[dataframe]") {
    TabularBundle df;
    REQUIRE(df.rowCount() == 0);
    REQUIRE(df.columnCount() == 0);
}

TEST_CASE("TabularBundle from columns", "[dataframe]") {
    TabularBundle df;
    df.addColumn(std::make_shared<Rank1Dataset>(QStringLiteral("a"), Unit::dimensionless(),
                                                 std::vector<int64_t>{1, 2, 3}));
    df.addColumn(std::make_shared<Rank1Dataset>(QStringLiteral("b"), Unit::dimensionless(),
                                                 std::vector<double>{1.1, 2.2, 3.3}));
    REQUIRE(df.rowCount() == 3);
    REQUIRE(df.columnCount() == 2);
}

TEST_CASE("TabularBundle column access by index", "[dataframe]") {
    TabularBundle df;
    df.addColumn(std::make_shared<Rank1Dataset>(QStringLiteral("x"), Unit::dimensionless(),
                                                 std::vector<int64_t>{10}));
    REQUIRE(df.column(0)->name() == QStringLiteral("x"));
    REQUIRE(df.column(1) == nullptr);
}

TEST_CASE("TabularBundle column lookup by name", "[dataframe]") {
    TabularBundle df;
    df.addColumn(std::make_shared<Rank1Dataset>(QStringLiteral("alpha"), Unit::dimensionless(),
                                                 std::vector<int64_t>{1}));
    df.addColumn(std::make_shared<Rank1Dataset>(QStringLiteral("beta"), Unit::dimensionless(),
                                                 std::vector<double>{2.0}));
    REQUIRE(df.columnByName(QStringLiteral("alpha")) != nullptr);
    CHECK_NOTHROW(df.columnByName(QStringLiteral("alpha"))->int64Data());
    REQUIRE(df.columnByName(QStringLiteral("beta")) != nullptr);
    REQUIRE(df.columnByName(QStringLiteral("gamma")) == nullptr);
}

TEST_CASE("TabularBundle rejects mismatched row counts", "[dataframe]") {
    TabularBundle df;
    df.addColumn(std::make_shared<Rank1Dataset>(QStringLiteral("a"), Unit::dimensionless(),
                                                 std::vector<int64_t>{1, 2}));
    REQUIRE_THROWS_AS(
        df.addColumn(std::make_shared<Rank1Dataset>(QStringLiteral("b"), Unit::dimensionless(),
                                                     std::vector<int64_t>{1})),
        std::invalid_argument);
}
