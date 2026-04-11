// Unit tests for DataFrameTableModel (backed by TabularBundle).

#include <catch2/catch_test_macros.hpp>

#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>
#include <data/Unit.h>
#include <style/DesignTokens.h>
#include <ui/DataFrameTableModel.h>

#include <QBrush>
#include <QColor>
#include <QString>
#include <QVariant>

#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

using namespace lumen::data;
using namespace lumen::ui;

namespace {

TabularBundle makeTestBundle() {
    TabularBundle bundle;
    bundle.addColumn(std::make_shared<Rank1Dataset>(QStringLiteral("id"), Unit::dimensionless(),
                                                     std::vector<int64_t>{1, 2, 3}));
    bundle.addColumn(std::make_shared<Rank1Dataset>(
        QStringLiteral("value"), Unit::dimensionless(),
        std::vector<double>{1.23456789, std::numeric_limits<double>::quiet_NaN(), 42.0}));
    bundle.addColumn(std::make_shared<Rank1Dataset>(
        QStringLiteral("label"), Unit::dimensionless(),
        std::vector<QString>{QStringLiteral("alpha"), QStringLiteral("beta"),
                             QStringLiteral("gamma")}));
    return bundle;
}

}  // namespace

TEST_CASE("DataFrameTableModel rowCount/columnCount match TabularBundle", "[table-model]") {
    auto bundle = makeTestBundle();
    DataFrameTableModel model;
    model.setDataFrame(&bundle);

    REQUIRE(model.rowCount() == 3);
    REQUIRE(model.columnCount() == 3);
}

TEST_CASE("DataFrameTableModel returns zero for null TabularBundle", "[table-model]") {
    DataFrameTableModel model;

    REQUIRE(model.rowCount() == 0);
    REQUIRE(model.columnCount() == 0);
}

TEST_CASE("DataFrameTableModel formats doubles to 6 significant digits", "[table-model]") {
    auto bundle = makeTestBundle();
    DataFrameTableModel model;
    model.setDataFrame(&bundle);

    auto index = model.index(0, 1);
    QString displayed = model.data(index, Qt::DisplayRole).toString();
    REQUIRE(displayed == QStringLiteral("1.23457"));
}

TEST_CASE("DataFrameTableModel displays NaN for NaN values", "[table-model]") {
    auto bundle = makeTestBundle();
    DataFrameTableModel model;
    model.setDataFrame(&bundle);

    auto index = model.index(1, 1);
    QString displayed = model.data(index, Qt::DisplayRole).toString();
    REQUIRE(displayed == QStringLiteral("NaN"));
}

TEST_CASE("DataFrameTableModel ForegroundRole returns grey for NaN", "[table-model]") {
    auto bundle = makeTestBundle();
    DataFrameTableModel model;
    model.setDataFrame(&bundle);

    auto nanIndex = model.index(1, 1);
    QVariant fg = model.data(nanIndex, Qt::ForegroundRole);
    REQUIRE(fg.isValid());
    QBrush brush = fg.value<QBrush>();
    REQUIRE(brush.color() == lumen::tokens::color::text::tertiary);

    auto normalIndex = model.index(0, 1);
    QVariant normalFg = model.data(normalIndex, Qt::ForegroundRole);
    REQUIRE_FALSE(normalFg.isValid());
}

TEST_CASE("DataFrameTableModel headerData returns column names", "[table-model]") {
    auto bundle = makeTestBundle();
    DataFrameTableModel model;
    model.setDataFrame(&bundle);

    REQUIRE(model.headerData(0, Qt::Horizontal).toString() == QStringLiteral("id"));
    REQUIRE(model.headerData(1, Qt::Horizontal).toString() == QStringLiteral("value"));
    REQUIRE(model.headerData(2, Qt::Horizontal).toString() == QStringLiteral("label"));
}

TEST_CASE("DataFrameTableModel displays int64 values as integers", "[table-model]") {
    auto bundle = makeTestBundle();
    DataFrameTableModel model;
    model.setDataFrame(&bundle);

    auto index = model.index(0, 0);
    QString displayed = model.data(index, Qt::DisplayRole).toString();
    REQUIRE(displayed == QStringLiteral("1"));
}

TEST_CASE("DataFrameTableModel displays string values as-is", "[table-model]") {
    auto bundle = makeTestBundle();
    DataFrameTableModel model;
    model.setDataFrame(&bundle);

    auto index = model.index(0, 2);
    QString displayed = model.data(index, Qt::DisplayRole).toString();
    REQUIRE(displayed == QStringLiteral("alpha"));
}

TEST_CASE("DataFrameTableModel TextAlignmentRole: numbers right, strings left",
          "[table-model]") {
    auto bundle = makeTestBundle();
    DataFrameTableModel model;
    model.setDataFrame(&bundle);

    auto intIndex = model.index(0, 0);
    int intAlign = model.data(intIndex, Qt::TextAlignmentRole).toInt();
    REQUIRE((intAlign & Qt::AlignRight) != 0);

    auto dblIndex = model.index(0, 1);
    int dblAlign = model.data(dblIndex, Qt::TextAlignmentRole).toInt();
    REQUIRE((dblAlign & Qt::AlignRight) != 0);

    auto strIndex = model.index(0, 2);
    int strAlign = model.data(strIndex, Qt::TextAlignmentRole).toInt();
    REQUIRE((strAlign & Qt::AlignLeft) != 0);
}

TEST_CASE("DataFrameTableModel vertical header shows 1-based row numbers", "[table-model]") {
    auto bundle = makeTestBundle();
    DataFrameTableModel model;
    model.setDataFrame(&bundle);

    REQUIRE(model.headerData(0, Qt::Vertical).toInt() == 1);
    REQUIRE(model.headerData(2, Qt::Vertical).toInt() == 3);
}
