// Unit tests for TabularBundle.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>
#include <data/Unit.h>

#include <memory>

using namespace lumen::data;
using Catch::Matchers::WithinRel;

TEST_CASE("TabularBundle empty", "[tabular]") {
    TabularBundle bundle;
    REQUIRE(bundle.columnCount() == 0);
    REQUIRE(bundle.rowCount() == 0);
    REQUIRE(bundle.columnNames().isEmpty());
}

TEST_CASE("TabularBundle add columns and access", "[tabular]") {
    auto col1 = std::make_shared<Rank1Dataset>(
        QStringLiteral("time"), Unit::parse(QStringLiteral("ms")),
        std::vector<double>{0.0, 1.0, 2.0});
    auto col2 = std::make_shared<Rank1Dataset>(
        QStringLiteral("voltage"), Unit::parse(QStringLiteral("mV")),
        std::vector<double>{10.0, 20.0, 30.0});

    TabularBundle bundle;
    bundle.addColumn(col1);
    bundle.addColumn(col2);

    REQUIRE(bundle.columnCount() == 2);
    REQUIRE(bundle.rowCount() == 3);
}

TEST_CASE("TabularBundle column by index", "[tabular]") {
    auto col = std::make_shared<Rank1Dataset>(
        QStringLiteral("data"), Unit::dimensionless(),
        std::vector<double>{1.0, 2.0});

    TabularBundle bundle;
    bundle.addColumn(col);

    REQUIRE(bundle.column(0) != nullptr);
    REQUIRE(bundle.column(0)->name() == QStringLiteral("data"));
    REQUIRE(bundle.column(1) == nullptr);
    REQUIRE(bundle.column(-1) == nullptr);
}

TEST_CASE("TabularBundle column by name", "[tabular]") {
    auto col1 = std::make_shared<Rank1Dataset>(
        QStringLiteral("alpha"), Unit::dimensionless(),
        std::vector<double>{1.0});
    auto col2 = std::make_shared<Rank1Dataset>(
        QStringLiteral("beta"), Unit::dimensionless(),
        std::vector<double>{2.0});

    TabularBundle bundle;
    bundle.addColumn(col1);
    bundle.addColumn(col2);

    REQUIRE(bundle.columnByName(QStringLiteral("alpha")) != nullptr);
    REQUIRE(bundle.columnByName(QStringLiteral("beta")) != nullptr);
    REQUIRE(bundle.columnByName(QStringLiteral("gamma")) == nullptr);
}

TEST_CASE("TabularBundle columnNames", "[tabular]") {
    auto col1 = std::make_shared<Rank1Dataset>(
        QStringLiteral("x"), Unit::dimensionless(),
        std::vector<double>{1.0, 2.0});
    auto col2 = std::make_shared<Rank1Dataset>(
        QStringLiteral("y"), Unit::dimensionless(),
        std::vector<double>{3.0, 4.0});

    TabularBundle bundle;
    bundle.addColumn(col1);
    bundle.addColumn(col2);

    auto names = bundle.columnNames();
    REQUIRE(names.size() == 2);
    REQUIRE(names[0] == QStringLiteral("x"));
    REQUIRE(names[1] == QStringLiteral("y"));
}

TEST_CASE("TabularBundle rejects mismatched row count", "[tabular]") {
    auto col1 = std::make_shared<Rank1Dataset>(
        QStringLiteral("a"), Unit::dimensionless(),
        std::vector<double>{1.0, 2.0});
    auto col2 = std::make_shared<Rank1Dataset>(
        QStringLiteral("b"), Unit::dimensionless(),
        std::vector<double>{1.0, 2.0, 3.0});

    TabularBundle bundle;
    bundle.addColumn(col1);
    REQUIRE_THROWS_AS(bundle.addColumn(col2), std::invalid_argument);
}

TEST_CASE("TabularBundle rejects null column", "[tabular]") {
    TabularBundle bundle;
    REQUIRE_THROWS_AS(bundle.addColumn(nullptr), std::invalid_argument);
}
