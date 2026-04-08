// Unit tests for DataFrameTableModel.

#include <catch2/catch_test_macros.hpp>

#include <data/Column.h>
#include <data/ColumnType.h>
#include <data/DataFrame.h>
#include <style/DesignTokens.h>
#include <ui/DataFrameTableModel.h>

#include <QBrush>
#include <QColor>
#include <QString>
#include <QVariant>

#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

using namespace lumen::data;
using namespace lumen::ui;

namespace {

/// Build a small test DataFrame with 3 columns: int64, double (with NaN), string.
DataFrame makeTestDataFrame() {
    std::vector<Column> cols;
    cols.emplace_back(QStringLiteral("id"), std::vector<int64_t>{1, 2, 3});
    cols.emplace_back(
        QStringLiteral("value"),
        std::vector<double>{1.23456789, std::numeric_limits<double>::quiet_NaN(), 42.0});
    cols.emplace_back(
        QStringLiteral("label"),
        std::vector<QString>{QStringLiteral("alpha"), QStringLiteral("beta"),
                             QStringLiteral("gamma")});
    return DataFrame(std::move(cols));
}

}  // namespace

TEST_CASE("DataFrameTableModel rowCount/columnCount match DataFrame", "[table-model]") {
    auto df = makeTestDataFrame();
    DataFrameTableModel model;
    model.setDataFrame(&df);

    REQUIRE(model.rowCount() == 3);
    REQUIRE(model.columnCount() == 3);
}

TEST_CASE("DataFrameTableModel returns zero for null DataFrame", "[table-model]") {
    DataFrameTableModel model;

    REQUIRE(model.rowCount() == 0);
    REQUIRE(model.columnCount() == 0);
}

TEST_CASE("DataFrameTableModel formats doubles to 6 significant digits", "[table-model]") {
    auto df = makeTestDataFrame();
    DataFrameTableModel model;
    model.setDataFrame(&df);

    // Column 1 ("value"), row 0 has 1.23456789
    auto index = model.index(0, 1);
    QString displayed = model.data(index, Qt::DisplayRole).toString();
    REQUIRE(displayed == QStringLiteral("1.23457"));
}

TEST_CASE("DataFrameTableModel displays NaN for NaN values", "[table-model]") {
    auto df = makeTestDataFrame();
    DataFrameTableModel model;
    model.setDataFrame(&df);

    // Column 1 ("value"), row 1 is NaN
    auto index = model.index(1, 1);
    QString displayed = model.data(index, Qt::DisplayRole).toString();
    REQUIRE(displayed == QStringLiteral("NaN"));
}

TEST_CASE("DataFrameTableModel ForegroundRole returns grey for NaN", "[table-model]") {
    auto df = makeTestDataFrame();
    DataFrameTableModel model;
    model.setDataFrame(&df);

    // NaN cell should have text.tertiary color
    auto nanIndex = model.index(1, 1);
    QVariant fg = model.data(nanIndex, Qt::ForegroundRole);
    REQUIRE(fg.isValid());
    QBrush brush = fg.value<QBrush>();
    REQUIRE(brush.color() == lumen::tokens::color::text::tertiary);

    // Non-NaN cell should have no foreground override
    auto normalIndex = model.index(0, 1);
    QVariant normalFg = model.data(normalIndex, Qt::ForegroundRole);
    REQUIRE_FALSE(normalFg.isValid());
}

TEST_CASE("DataFrameTableModel headerData returns column names", "[table-model]") {
    auto df = makeTestDataFrame();
    DataFrameTableModel model;
    model.setDataFrame(&df);

    REQUIRE(model.headerData(0, Qt::Horizontal).toString() == QStringLiteral("id"));
    REQUIRE(model.headerData(1, Qt::Horizontal).toString() == QStringLiteral("value"));
    REQUIRE(model.headerData(2, Qt::Horizontal).toString() == QStringLiteral("label"));
}

TEST_CASE("DataFrameTableModel displays int64 values as integers", "[table-model]") {
    auto df = makeTestDataFrame();
    DataFrameTableModel model;
    model.setDataFrame(&df);

    auto index = model.index(0, 0);
    QString displayed = model.data(index, Qt::DisplayRole).toString();
    REQUIRE(displayed == QStringLiteral("1"));
}

TEST_CASE("DataFrameTableModel displays string values as-is", "[table-model]") {
    auto df = makeTestDataFrame();
    DataFrameTableModel model;
    model.setDataFrame(&df);

    auto index = model.index(0, 2);
    QString displayed = model.data(index, Qt::DisplayRole).toString();
    REQUIRE(displayed == QStringLiteral("alpha"));
}

TEST_CASE("DataFrameTableModel TextAlignmentRole: numbers right, strings left",
          "[table-model]") {
    auto df = makeTestDataFrame();
    DataFrameTableModel model;
    model.setDataFrame(&df);

    // Int column — right aligned
    auto intIndex = model.index(0, 0);
    int intAlign = model.data(intIndex, Qt::TextAlignmentRole).toInt();
    REQUIRE((intAlign & Qt::AlignRight) != 0);

    // Double column — right aligned
    auto dblIndex = model.index(0, 1);
    int dblAlign = model.data(dblIndex, Qt::TextAlignmentRole).toInt();
    REQUIRE((dblAlign & Qt::AlignRight) != 0);

    // String column — left aligned
    auto strIndex = model.index(0, 2);
    int strAlign = model.data(strIndex, Qt::TextAlignmentRole).toInt();
    REQUIRE((strAlign & Qt::AlignLeft) != 0);
}

TEST_CASE("DataFrameTableModel vertical header shows 1-based row numbers", "[table-model]") {
    auto df = makeTestDataFrame();
    DataFrameTableModel model;
    model.setDataFrame(&df);

    REQUIRE(model.headerData(0, Qt::Vertical).toInt() == 1);
    REQUIRE(model.headerData(2, Qt::Vertical).toInt() == 3);
}
