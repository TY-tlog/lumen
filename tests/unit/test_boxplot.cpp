// Unit tests for BoxPlotSeries — T17.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "data/Rank1Dataset.h"
#include "data/Unit.h"
#include "plot/BoxPlotSeries.h"
#include "plot/CoordinateMapper.h"

#include <QGuiApplication>
#include <QImage>
#include <QPainter>

#include <cmath>
#include <memory>
#include <vector>

using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using lumen::plot::BoxPlotSeries;
using lumen::plot::CoordinateMapper;
using Catch::Matchers::WithinAbs;

namespace {

// Ensure QGuiApplication exists for QPainter/QImage.
struct AppGuard {
    AppGuard() {
        if (QGuiApplication::instance() == nullptr) {
            static int argc = 1;
            static char arg0[] = "test";
            static char* argv[] = {arg0, nullptr};
            app = new QGuiApplication(argc, argv);
        }
    }
    QGuiApplication* app = nullptr;
};
static AppGuard guard;

/// Check if image has any non-white pixels.
bool hasNonWhitePixels(const QImage& image) {
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y) != QColor(Qt::white)) {
                return true;
            }
        }
    }
    return false;
}

} // anonymous namespace

TEST_CASE("BoxPlotSeries: type returns BoxPlot", "[boxplot]")
{
    auto ds = std::make_shared<Rank1Dataset>(
        "vals", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0});
    BoxPlotSeries bp(ds);
    REQUIRE(bp.type() == lumen::plot::PlotItem::Type::BoxPlot);
}

TEST_CASE("BoxPlotSeries: quartiles correct on known data", "[boxplot]")
{
    // Data: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
    // Sorted: same. n=12.
    // Q1 (25th percentile): index = 0.25 * 11 = 2.75 -> lerp(3, 4, 0.75) = 3.75
    // Median (50th percentile): index = 0.5 * 11 = 5.5 -> lerp(6, 7, 0.5) = 6.5
    // Q3 (75th percentile): index = 0.75 * 11 = 8.25 -> lerp(9, 10, 0.25) = 9.25
    std::vector<double> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    BoxPlotSeries bp(ds);
    auto stats = bp.computeStats();

    REQUIRE_THAT(stats.median, WithinAbs(6.5, 1e-9));
    REQUIRE_THAT(stats.q1, WithinAbs(3.75, 1e-9));
    REQUIRE_THAT(stats.q3, WithinAbs(9.25, 1e-9));
}

TEST_CASE("BoxPlotSeries: Tukey whiskers and outliers", "[boxplot]")
{
    // Data with clear outliers.
    // Main cluster: 10, 20, 30, 40, 50 (Q1=15, Q3=45, IQR=30)
    // Tukey fences: Q1 - 1.5*IQR = -30, Q3 + 1.5*IQR = 90
    // Add outlier at 100 and -50.
    std::vector<double> data = {-50, 10, 20, 30, 40, 50, 100};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    BoxPlotSeries bp(ds);
    bp.setWhiskerRule(BoxPlotSeries::WhiskerRule::Tukey);

    auto stats = bp.computeStats();

    // IQR = Q3 - Q1
    double iqr = stats.q3 - stats.q1;
    double lowerFence = stats.q1 - 1.5 * iqr;
    double upperFence = stats.q3 + 1.5 * iqr;

    // Whiskers should be within fences.
    REQUIRE(stats.whiskerLo >= lowerFence - 1e-9);
    REQUIRE(stats.whiskerHi <= upperFence + 1e-9);

    // -50 should be an outlier (below lower fence).
    bool foundLowOutlier = false;
    for (double o : stats.outliers) {
        if (std::abs(o - (-50.0)) < 1e-9) {
            foundLowOutlier = true;
        }
    }
    // Whether -50 is an outlier depends on Q1/Q3 with 7 data points.
    // n=7, sorted: -50, 10, 20, 30, 40, 50, 100
    // Q1 = percentile(25) = index 1.5 -> lerp(10, 20, 0.5) = 15
    // Q3 = percentile(75) = index 4.5 -> lerp(40, 50, 0.5) = 45
    // IQR = 30
    // Lower fence = 15 - 45 = -30. -50 < -30 -> outlier.
    // Upper fence = 45 + 45 = 90. 100 > 90 -> outlier.
    REQUIRE(foundLowOutlier);

    bool foundHighOutlier = false;
    for (double o : stats.outliers) {
        if (std::abs(o - 100.0) < 1e-9) {
            foundHighOutlier = true;
        }
    }
    REQUIRE(foundHighOutlier);
}

TEST_CASE("BoxPlotSeries: MinMax whiskers have no outliers", "[boxplot]")
{
    std::vector<double> data = {1, 2, 3, 4, 5, 100};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    BoxPlotSeries bp(ds);
    bp.setWhiskerRule(BoxPlotSeries::WhiskerRule::MinMax);

    auto stats = bp.computeStats();

    REQUIRE_THAT(stats.whiskerLo, WithinAbs(1.0, 1e-9));
    REQUIRE_THAT(stats.whiskerHi, WithinAbs(100.0, 1e-9));
    REQUIRE(stats.outliers.empty());
}

TEST_CASE("BoxPlotSeries: Percentile whiskers at 5th/95th", "[boxplot]")
{
    // 100 values from 0 to 99.
    std::vector<double> data(100);
    for (int i = 0; i < 100; ++i) {
        data[static_cast<std::size_t>(i)] = static_cast<double>(i);
    }
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    BoxPlotSeries bp(ds);
    bp.setWhiskerRule(BoxPlotSeries::WhiskerRule::Percentile);

    auto stats = bp.computeStats();

    // 5th percentile of 0..99 (n=100): index = 0.05 * 99 = 4.95
    REQUIRE_THAT(stats.whiskerLo, WithinAbs(4.95, 0.5));
    // 95th percentile: index = 0.95 * 99 = 94.05
    REQUIRE_THAT(stats.whiskerHi, WithinAbs(94.05, 0.5));
}

TEST_CASE("BoxPlotSeries: empty data produces zero stats", "[boxplot]")
{
    auto ds = std::make_shared<Rank1Dataset>(
        "vals", Unit::dimensionless(), std::vector<double>{});

    BoxPlotSeries bp(ds);
    auto stats = bp.computeStats();

    REQUIRE_THAT(stats.median, WithinAbs(0.0, 1e-9));
    REQUIRE_THAT(stats.q1, WithinAbs(0.0, 1e-9));
    REQUIRE_THAT(stats.q3, WithinAbs(0.0, 1e-9));
    REQUIRE(stats.outliers.empty());
}

TEST_CASE("BoxPlotSeries: paint renders", "[boxplot]")
{
    std::vector<double> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    BoxPlotSeries bp(ds);
    bp.setPosition(5.0);
    bp.setBoxWidth(2.0);
    bp.setFillColor(Qt::cyan);

    QImage canvas(200, 200, QImage::Format_ARGB32);
    canvas.fill(Qt::white);

    CoordinateMapper mapper(0.0, 10.0, 0.0, 12.0, QRectF(10, 10, 180, 180));
    QRectF plotArea(10, 10, 180, 180);

    QPainter painter(&canvas);
    bp.paint(&painter, mapper, plotArea);
    painter.end();

    REQUIRE(hasNonWhitePixels(canvas));
}

TEST_CASE("BoxPlotSeries: notched mode", "[boxplot]")
{
    std::vector<double> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    BoxPlotSeries bp(ds);
    bp.setNotched(true);
    bp.setPosition(5.0);
    bp.setBoxWidth(2.0);

    REQUIRE(bp.notched());

    QImage canvas(200, 200, QImage::Format_ARGB32);
    canvas.fill(Qt::white);

    CoordinateMapper mapper(0.0, 10.0, 0.0, 12.0, QRectF(10, 10, 180, 180));
    QRectF plotArea(10, 10, 180, 180);

    QPainter painter(&canvas);
    bp.paint(&painter, mapper, plotArea);
    painter.end();

    REQUIRE(hasNonWhitePixels(canvas));
}

TEST_CASE("BoxPlotSeries: null data throws", "[boxplot]")
{
    REQUIRE_THROWS_AS(BoxPlotSeries(nullptr), std::invalid_argument);
}
