// BarSeries tests -- bar rendering, layout, and data bounds.
// QGuiApplication + fontconfig triggers benign LeakSanitizer
// reports from libfontconfig internals (not our code).

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "data/Rank1Dataset.h"
#include "data/Unit.h"
#include "plot/BarSeries.h"
#include "plot/CoordinateMapper.h"

#include <QGuiApplication>
#include <QImage>
#include <QPainter>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using lumen::plot::BarSeries;
using lumen::plot::CoordinateMapper;
using Catch::Matchers::WithinAbs;

namespace {

const auto kNaN = std::numeric_limits<double>::quiet_NaN();

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

/// Render a bar series to a QImage and return it.
QImage renderBars(const BarSeries& series) {
    QImage image(200, 200, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    QRectF plotArea(10, 10, 180, 180);
    CoordinateMapper mapper(0.0, 10.0, -5.0, 10.0, plotArea);
    series.paint(&painter, mapper, plotArea);
    painter.end();

    return image;
}

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

} // namespace

TEST_CASE("BarSeries: paint produces bars",
          "[plot][bar_series]")
{
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 3.0, 5.0, 7.0, 9.0});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{2.0, 4.0, 6.0, 8.0, 3.0});

    BarSeries series(xCol, yCol, Qt::blue, "bars");
    QImage image = renderBars(series);

    REQUIRE(hasNonWhitePixels(image));
}

TEST_CASE("BarSeries: NaN skipped",
          "[plot][bar_series]")
{
    // All NaN should produce an empty (all-white) image.
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{kNaN, kNaN, kNaN});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{kNaN, kNaN, kNaN});

    BarSeries series(xCol, yCol, Qt::blue, "nan_bars");
    QImage image = renderBars(series);

    CHECK_FALSE(hasNonWhitePixels(image));
}

TEST_CASE("BarSeries: partial NaN renders only valid bars",
          "[plot][bar_series]")
{
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{2.0, kNaN, 8.0});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{5.0, 5.0, 5.0});

    BarSeries series(xCol, yCol, Qt::green, "partial");
    QImage image = renderBars(series);

    // Should have content from the two valid bars.
    REQUIRE(hasNonWhitePixels(image));
}

TEST_CASE("BarSeries: dataBounds includes y=0 baseline",
          "[plot][bar_series]")
{
    // All positive Y values -- yMin should still be 0.
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 3.0, 5.0});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{10.0, 20.0, 30.0});

    BarSeries series(xCol, yCol, Qt::red);
    QRectF bounds = series.dataBounds();

    CHECK_THAT(bounds.left(), WithinAbs(1.0, 1e-10));
    CHECK_THAT(bounds.right(), WithinAbs(5.0, 1e-10));
    // y=0 must be included as baseline.
    CHECK_THAT(bounds.top(), WithinAbs(0.0, 1e-10));
    CHECK_THAT(bounds.bottom(), WithinAbs(30.0, 1e-10));
}

TEST_CASE("BarSeries: dataBounds with negative y includes both 0 and negative",
          "[plot][bar_series]")
{
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 3.0});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{-5.0, 10.0});

    BarSeries series(xCol, yCol, Qt::red);
    QRectF bounds = series.dataBounds();

    CHECK_THAT(bounds.top(), WithinAbs(-5.0, 1e-10));
    CHECK_THAT(bounds.bottom(), WithinAbs(10.0, 1e-10));
}

TEST_CASE("BarSeries: dataBounds with all NaN returns zero rect",
          "[plot][bar_series]")
{
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{kNaN, kNaN});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{kNaN, kNaN});

    BarSeries series(xCol, yCol, Qt::blue);
    QRectF bounds = series.dataBounds();

    CHECK_THAT(bounds.left(), WithinAbs(0.0, 1e-10));
    CHECK_THAT(bounds.top(), WithinAbs(0.0, 1e-10));
    CHECK_THAT(bounds.width(), WithinAbs(0.0, 1e-10));
    CHECK_THAT(bounds.height(), WithinAbs(0.0, 1e-10));
}

TEST_CASE("BarSeries: single point -- fallback bar width (2px min)",
          "[plot][bar_series]")
{
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{5.0});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{8.0});

    BarSeries series(xCol, yCol, Qt::red, "single");
    QImage image = renderBars(series);

    // Single point should still produce a visible bar (min 2px width).
    REQUIRE(hasNonWhitePixels(image));
}

TEST_CASE("BarSeries: setters work",
          "[plot][bar_series]")
{
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 2.0});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{3.0, 4.0});

    BarSeries series(xCol, yCol, Qt::red, "original");

    series.setFillColor(Qt::blue);
    CHECK(series.fillColor() == QColor(Qt::blue));
    CHECK(series.primaryColor() == QColor(Qt::blue));

    series.setOutlineColor(Qt::black);
    CHECK(series.outlineColor() == QColor(Qt::black));

    series.setBarWidth(0.5);
    CHECK_THAT(series.barWidth(), WithinAbs(0.5, 1e-10));

    // Clamping: below minimum.
    series.setBarWidth(0.01);
    CHECK_THAT(series.barWidth(), WithinAbs(0.1, 1e-10));

    // Clamping: above maximum.
    series.setBarWidth(2.0);
    CHECK_THAT(series.barWidth(), WithinAbs(1.0, 1e-10));

    series.setName("renamed");
    CHECK(series.name() == "renamed");

    series.setVisible(false);
    CHECK_FALSE(series.isVisible());
}

TEST_CASE("BarSeries: invisible hides",
          "[plot][bar_series]")
{
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 3.0, 5.0, 7.0, 9.0});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{2.0, 4.0, 6.0, 8.0, 3.0});

    BarSeries series(xCol, yCol, Qt::blue, "hidden");
    series.setVisible(false);

    QImage image = renderBars(series);
    CHECK_FALSE(hasNonWhitePixels(image));
}

TEST_CASE("BarSeries: outline renders when set",
          "[plot][bar_series]")
{
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{5.0});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{8.0});

    BarSeries series(xCol, yCol, Qt::blue, "outlined");
    series.setOutlineColor(Qt::black);
    series.setBarWidth(0.8);

    QImage image = renderBars(series);
    REQUIRE(hasNonWhitePixels(image));
}

TEST_CASE("BarSeries: type returns Bar",
          "[plot][bar_series]")
{
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0});
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{1.0});

    BarSeries series(xCol, yCol, Qt::red);
    CHECK(series.type() == lumen::plot::PlotItem::Type::Bar);
}
