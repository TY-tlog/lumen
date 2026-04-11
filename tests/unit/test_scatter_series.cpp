// ScatterSeries tests -- marker rendering and data bounds.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "data/Rank1Dataset.h"
#include "data/Unit.h"
#include "plot/CoordinateMapper.h"
#include "plot/ScatterSeries.h"

#include <QGuiApplication>
#include <QImage>
#include <QPainter>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using lumen::plot::CoordinateMapper;
using lumen::plot::MarkerShape;
using lumen::plot::ScatterSeries;
using Catch::Matchers::WithinAbs;

namespace {

const auto kNaN = std::numeric_limits<double>::quiet_NaN();

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

QImage renderScatter(const ScatterSeries& series) {
    QImage image(200, 200, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    QRectF plotArea(10, 10, 180, 180);
    CoordinateMapper mapper(0.0, 10.0, 0.0, 10.0, plotArea);
    series.paint(&painter, mapper, plotArea);
    painter.end();
    return image;
}

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

TEST_CASE("ScatterSeries: paint produces non-empty image", "[plot][scatter_series]") {
    auto xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 3.0, 5.0, 7.0, 9.0});
    auto yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{2.0, 4.0, 6.0, 8.0, 1.0});
    ScatterSeries series(xDs, yDs, Qt::red, "scatter");
    REQUIRE(hasNonWhitePixels(renderScatter(series)));
}

TEST_CASE("ScatterSeries: NaN skipped in paint", "[plot][scatter_series]") {
    auto xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{kNaN, kNaN, kNaN});
    auto yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{kNaN, kNaN, kNaN});
    ScatterSeries series(xDs, yDs, Qt::blue, "nan_test");
    CHECK_FALSE(hasNonWhitePixels(renderScatter(series)));
}

TEST_CASE("ScatterSeries: partial NaN renders only valid points", "[plot][scatter_series]") {
    auto xFull = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{2.0, 5.0, 8.0});
    auto yFull = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{2.0, 5.0, 8.0});
    ScatterSeries fullSeries(xFull, yFull, Qt::red, "full");

    auto xPartial = std::make_shared<Rank1Dataset>("x2", Unit::dimensionless(), std::vector<double>{2.0, kNaN, 8.0});
    auto yPartial = std::make_shared<Rank1Dataset>("y2", Unit::dimensionless(), std::vector<double>{2.0, 5.0, 8.0});
    ScatterSeries partialSeries(xPartial, yPartial, Qt::red, "partial");

    REQUIRE(hasNonWhitePixels(renderScatter(fullSeries)));
    REQUIRE(hasNonWhitePixels(renderScatter(partialSeries)));
}

TEST_CASE("ScatterSeries: dataBounds correct", "[plot][scatter_series]") {
    auto xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, kNaN, 5.0, 3.0});
    auto yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{10.0, 20.0, kNaN, 40.0});
    ScatterSeries series(xDs, yDs, Qt::green);
    QRectF bounds = series.dataBounds();
    CHECK_THAT(bounds.left(), WithinAbs(1.0, 1e-10));
    CHECK_THAT(bounds.right(), WithinAbs(3.0, 1e-10));
    CHECK_THAT(bounds.top(), WithinAbs(10.0, 1e-10));
    CHECK_THAT(bounds.bottom(), WithinAbs(40.0, 1e-10));
}

TEST_CASE("ScatterSeries: dataBounds with all NaN returns zero rect", "[plot][scatter_series]") {
    auto xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{kNaN, kNaN});
    auto yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{kNaN, kNaN});
    ScatterSeries series(xDs, yDs, Qt::blue);
    QRectF bounds = series.dataBounds();
    CHECK_THAT(bounds.left(), WithinAbs(0.0, 1e-10));
    CHECK_THAT(bounds.top(), WithinAbs(0.0, 1e-10));
    CHECK_THAT(bounds.width(), WithinAbs(0.0, 1e-10));
    CHECK_THAT(bounds.height(), WithinAbs(0.0, 1e-10));
}

TEST_CASE("ScatterSeries: setters work", "[plot][scatter_series]") {
    auto xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 2.0});
    auto yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{3.0, 4.0});
    ScatterSeries series(xDs, yDs, Qt::red, "original");

    series.setColor(Qt::blue);
    CHECK(series.color() == QColor(Qt::blue));
    CHECK(series.primaryColor() == QColor(Qt::blue));
    series.setMarkerShape(MarkerShape::Diamond);
    CHECK(series.markerShape() == MarkerShape::Diamond);
    series.setMarkerSize(12);
    CHECK(series.markerSize() == 12);
    series.setMarkerSize(1);
    CHECK(series.markerSize() == 3);
    series.setMarkerSize(50);
    CHECK(series.markerSize() == 20);
    series.setFilled(false);
    CHECK_FALSE(series.filled());
    series.setName("renamed");
    CHECK(series.name() == "renamed");
    series.setVisible(false);
    CHECK_FALSE(series.isVisible());
}

TEST_CASE("ScatterSeries: invisible series produces empty render", "[plot][scatter_series]") {
    auto xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0, 3.0, 5.0, 7.0, 9.0});
    auto yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{2.0, 4.0, 6.0, 8.0, 1.0});
    ScatterSeries series(xDs, yDs, Qt::red, "hidden");
    series.setVisible(false);
    CHECK_FALSE(hasNonWhitePixels(renderScatter(series)));
}

TEST_CASE("ScatterSeries: all marker shapes render", "[plot][scatter_series]") {
    auto xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{5.0});
    auto yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{5.0});
    const MarkerShape shapes[] = {
        MarkerShape::Circle, MarkerShape::Square, MarkerShape::Triangle,
        MarkerShape::Diamond, MarkerShape::Plus, MarkerShape::Cross
    };
    for (auto shape : shapes) {
        ScatterSeries series(xDs, yDs, Qt::red, "marker_test");
        series.setMarkerShape(shape);
        series.setMarkerSize(10);
        REQUIRE(hasNonWhitePixels(renderScatter(series)));
    }
}

TEST_CASE("ScatterSeries: type returns Scatter", "[plot][scatter_series]") {
    auto xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0});
    auto yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{1.0});
    ScatterSeries series(xDs, yDs, Qt::red);
    CHECK(series.type() == lumen::plot::PlotItem::Type::Scatter);
}
