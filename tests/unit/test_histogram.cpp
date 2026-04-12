// Unit tests for HistogramSeries — T16.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "data/Rank1Dataset.h"
#include "data/Unit.h"
#include "plot/CoordinateMapper.h"
#include "plot/HistogramSeries.h"

#include <QGuiApplication>
#include <QImage>
#include <QPainter>

#include <cmath>
#include <memory>
#include <numeric>
#include <vector>

using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using lumen::plot::CoordinateMapper;
using lumen::plot::HistogramSeries;
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

TEST_CASE("HistogramSeries: type returns Histogram", "[histogram]")
{
    auto ds = std::make_shared<Rank1Dataset>(
        "vals", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0});
    HistogramSeries hs(ds);
    REQUIRE(hs.type() == lumen::plot::PlotItem::Type::Histogram);
}

TEST_CASE("HistogramSeries: Sturges bin count", "[histogram]")
{
    // 32 data points: Sturges gives ceil(log2(32)) + 1 = 5 + 1 = 6 bins.
    std::vector<double> data(32);
    for (int i = 0; i < 32; ++i) {
        data[static_cast<std::size_t>(i)] = static_cast<double>(i);
    }
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    HistogramSeries hs(ds);
    hs.setAutoBinning(HistogramSeries::BinRule::Sturges);

    int bins = hs.computeAutoBinCount();
    REQUIRE(bins == 6);
}

TEST_CASE("HistogramSeries: Scott bin rule", "[histogram]")
{
    // 100 values from 0 to 99.
    std::vector<double> data(100);
    for (int i = 0; i < 100; ++i) {
        data[static_cast<std::size_t>(i)] = static_cast<double>(i);
    }
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    HistogramSeries hs(ds);
    hs.setAutoBinning(HistogramSeries::BinRule::Scott);

    int bins = hs.computeAutoBinCount();
    REQUIRE(bins > 0);
    REQUIRE(bins < 100); // Scott should give far fewer bins than data points.
}

TEST_CASE("HistogramSeries: FreedmanDiaconis bin rule", "[histogram]")
{
    std::vector<double> data(200);
    for (int i = 0; i < 200; ++i) {
        data[static_cast<std::size_t>(i)] = static_cast<double>(i);
    }
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    HistogramSeries hs(ds);
    hs.setAutoBinning(HistogramSeries::BinRule::FreedmanDiaconis);

    int bins = hs.computeAutoBinCount();
    REQUIRE(bins > 0);
    REQUIRE(bins < 200);
}

TEST_CASE("HistogramSeries: manual bin count", "[histogram]")
{
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    HistogramSeries hs(ds);
    hs.setBinCount(5);

    auto bins = hs.computeBins();
    REQUIRE(bins.size() == 5);

    // Sum of counts should equal total data points.
    double totalCount = 0.0;
    for (const auto& b : bins) {
        totalCount += b.value;
    }
    REQUIRE_THAT(totalCount, WithinAbs(10.0, 1e-9));
}

TEST_CASE("HistogramSeries: probability normalization sums to 1", "[histogram]")
{
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    HistogramSeries hs(ds);
    hs.setBinCount(3);
    hs.setNormalization(HistogramSeries::Normalization::Probability);

    auto bins = hs.computeBins();
    double total = 0.0;
    for (const auto& b : bins) {
        total += b.value;
    }
    REQUIRE_THAT(total, WithinAbs(1.0, 1e-9));
}

TEST_CASE("HistogramSeries: density normalization integrates to 1", "[histogram]")
{
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    HistogramSeries hs(ds);
    hs.setBinCount(4);
    hs.setNormalization(HistogramSeries::Normalization::Density);

    auto bins = hs.computeBins();

    // Integral of density = sum(density * bin_width) should be ~1.0.
    double integral = 0.0;
    for (const auto& b : bins) {
        integral += b.value * (b.hi - b.lo);
    }
    REQUIRE_THAT(integral, WithinAbs(1.0, 1e-9));
}

TEST_CASE("HistogramSeries: empty data produces no bins", "[histogram]")
{
    auto ds = std::make_shared<Rank1Dataset>(
        "vals", Unit::dimensionless(), std::vector<double>{});

    HistogramSeries hs(ds);
    auto bins = hs.computeBins();
    REQUIRE(bins.empty());

    // dataBounds should also be empty.
    QRectF bounds = hs.dataBounds();
    REQUIRE(bounds.isEmpty());
}

TEST_CASE("HistogramSeries: paint produces bars", "[histogram]")
{
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    HistogramSeries hs(ds);
    hs.setBinCount(3);
    hs.setFillColor(Qt::blue);

    QImage canvas(200, 200, QImage::Format_ARGB32);
    canvas.fill(Qt::white);

    CoordinateMapper mapper(0.0, 6.0, 0.0, 5.0, QRectF(10, 10, 180, 180));
    QRectF plotArea(10, 10, 180, 180);

    QPainter painter(&canvas);
    hs.paint(&painter, mapper, plotArea);
    painter.end();

    REQUIRE(hasNonWhitePixels(canvas));
}

TEST_CASE("HistogramSeries: null data throws", "[histogram]")
{
    REQUIRE_THROWS_AS(HistogramSeries(nullptr), std::invalid_argument);
}
