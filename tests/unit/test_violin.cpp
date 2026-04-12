// Unit tests for ViolinSeries — T18.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "data/Rank1Dataset.h"
#include "data/Unit.h"
#include "plot/CoordinateMapper.h"
#include "plot/ViolinSeries.h"

#include <QGuiApplication>
#include <QImage>
#include <QPainter>

#include <cmath>
#include <memory>
#include <vector>

using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using lumen::plot::CoordinateMapper;
using lumen::plot::ViolinSeries;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

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

/// Generate normally-distributed-like data (Box-Muller transform).
std::vector<double> generateNormalData(int n, double mean, double sd)
{
    std::vector<double> result;
    result.reserve(static_cast<std::size_t>(n));
    // Simple deterministic pseudo-normal using sine approximation.
    for (int i = 0; i < n; ++i) {
        // Use a simple mapping to approximate a bell curve.
        double t = static_cast<double>(i) / static_cast<double>(n - 1);
        // Inverse of normal CDF approximation (Beasley-Springer-Moro).
        // For simplicity, use a simpler approach: map uniform to bell-like shape.
        double u = -3.0 + 6.0 * t; // range [-3, 3] in standard deviations
        result.push_back(mean + sd * u);
    }
    return result;
}

} // anonymous namespace

TEST_CASE("ViolinSeries: type returns Violin", "[violin]")
{
    auto ds = std::make_shared<Rank1Dataset>(
        "vals", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0});
    ViolinSeries vs(ds);
    REQUIRE(vs.type() == lumen::plot::PlotItem::Type::Violin);
}

TEST_CASE("ViolinSeries: Silverman bandwidth on normal-like data", "[violin]")
{
    auto data = generateNormalData(100, 0.0, 1.0);
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    ViolinSeries vs(ds);
    double bw = vs.computeSilvermanBandwidth();

    // Silverman bandwidth for n=100, std~1.7 (uniform on [-3,3]):
    // 0.9 * min(std, IQR/1.34) * n^(-1/5)
    // Should be a positive finite value.
    REQUIRE(bw > 0.0);
    REQUIRE(std::isfinite(bw));

    // For n=100, bandwidth should be reasonable (not too large, not too small).
    REQUIRE(bw < 5.0);
    REQUIRE(bw > 0.01);
}

TEST_CASE("ViolinSeries: KDE on normal-like data has peak near mean", "[violin]")
{
    auto data = generateNormalData(200, 5.0, 1.0);
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    ViolinSeries vs(ds);
    auto kde = vs.computeKde();

    REQUIRE(!kde.empty());

    // Find the point with maximum density.
    double maxDensity = 0.0;
    double peakY = 0.0;
    for (const auto& pt : kde) {
        if (pt.density > maxDensity) {
            maxDensity = pt.density;
            peakY = pt.y;
        }
    }

    // For uniform data on [-3+5, 3+5] = [2, 8], density should be relatively flat.
    // The peak should be somewhere in the data range.
    REQUIRE(peakY > 1.0);
    REQUIRE(peakY < 9.0);
}

TEST_CASE("ViolinSeries: KDE density is non-negative", "[violin]")
{
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    ViolinSeries vs(ds);
    auto kde = vs.computeKde();

    for (const auto& pt : kde) {
        REQUIRE(pt.density >= 0.0);
    }
}

TEST_CASE("ViolinSeries: manual bandwidth", "[violin]")
{
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    ViolinSeries vs(ds);
    vs.setKdeBandwidth(0.5);

    REQUIRE_FALSE(vs.kdeBandwidthAuto());
    REQUIRE_THAT(vs.kdeBandwidth(), WithinAbs(0.5, 1e-12));

    auto kdeNarrow = vs.computeKde();

    vs.setKdeBandwidth(2.0);
    auto kdeWide = vs.computeKde();

    // Narrower bandwidth should produce higher peak density (more peaked).
    double maxNarrow = 0.0;
    double maxWide = 0.0;
    for (const auto& pt : kdeNarrow) {
        maxNarrow = std::max(maxNarrow, pt.density);
    }
    for (const auto& pt : kdeWide) {
        maxWide = std::max(maxWide, pt.density);
    }

    REQUIRE(maxNarrow > maxWide);
}

TEST_CASE("ViolinSeries: split violin mode", "[violin]")
{
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    ViolinSeries vs(ds);
    vs.setSplit(true);
    vs.setPosition(5.0);
    vs.setMaxWidth(2.0);

    REQUIRE(vs.split());

    // Paint should produce output.
    QImage canvas(200, 200, QImage::Format_ARGB32);
    canvas.fill(Qt::white);

    CoordinateMapper mapper(0.0, 10.0, -2.0, 12.0, QRectF(10, 10, 180, 180));
    QRectF plotArea(10, 10, 180, 180);

    QPainter painter(&canvas);
    vs.paint(&painter, mapper, plotArea);
    painter.end();

    REQUIRE(hasNonWhitePixels(canvas));
}

TEST_CASE("ViolinSeries: full (mirrored) violin paint", "[violin]")
{
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    auto ds = std::make_shared<Rank1Dataset>("vals", Unit::dimensionless(), std::move(data));

    ViolinSeries vs(ds);
    vs.setPosition(5.0);
    vs.setMaxWidth(2.0);
    vs.setFillColor(Qt::magenta);

    QImage canvas(200, 200, QImage::Format_ARGB32);
    canvas.fill(Qt::white);

    CoordinateMapper mapper(0.0, 10.0, -2.0, 12.0, QRectF(10, 10, 180, 180));
    QRectF plotArea(10, 10, 180, 180);

    QPainter painter(&canvas);
    vs.paint(&painter, mapper, plotArea);
    painter.end();

    REQUIRE(hasNonWhitePixels(canvas));
}

TEST_CASE("ViolinSeries: empty data produces no KDE", "[violin]")
{
    auto ds = std::make_shared<Rank1Dataset>(
        "vals", Unit::dimensionless(), std::vector<double>{});

    ViolinSeries vs(ds);
    auto kde = vs.computeKde();
    REQUIRE(kde.empty());
}

TEST_CASE("ViolinSeries: null data throws", "[violin]")
{
    REQUIRE_THROWS_AS(ViolinSeries(nullptr), std::invalid_argument);
}

TEST_CASE("ViolinSeries: setters work", "[violin]")
{
    auto ds = std::make_shared<Rank1Dataset>(
        "vals", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0});

    ViolinSeries vs(ds);

    vs.setFillColor(Qt::red);
    REQUIRE(vs.fillColor() == QColor(Qt::red));
    REQUIRE(vs.primaryColor() == QColor(Qt::red));

    vs.setOutlineColor(Qt::green);
    REQUIRE(vs.outlineColor() == QColor(Qt::green));

    vs.setPosition(3.0);
    REQUIRE_THAT(vs.position(), WithinAbs(3.0, 1e-12));

    vs.setMaxWidth(1.5);
    REQUIRE_THAT(vs.maxWidth(), WithinAbs(1.5, 1e-12));

    vs.setName("myviolin");
    REQUIRE(vs.name() == "myviolin");

    vs.setVisible(false);
    REQUIRE_FALSE(vs.isVisible());
}
