// Unit tests for ContourPlot PlotItem — T13.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Unit.h>
#include <plot/Colormap.h>
#include <plot/ContourPlot.h>
#include <plot/CoordinateMapper.h>
#include <plot/Heatmap.h>

#include <QGuiApplication>
#include <QImage>
#include <QPainter>

#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace lumen::data;
using namespace lumen::plot;
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

/// Helper: create a Gaussian grid.
std::shared_ptr<Grid2D> makeGaussianGrid(std::size_t size)
{
    Dimension dimX{
        QStringLiteral("x"),
        Unit::dimensionless(),
        size,
        CoordinateArray(0.0, 1.0, size),
    };
    Dimension dimY{
        QStringLiteral("y"),
        Unit::dimensionless(),
        size,
        CoordinateArray(0.0, 1.0, size),
    };

    double cx = static_cast<double>(size - 1) / 2.0;
    double cy = cx;
    double sigma = static_cast<double>(size) / 5.0;
    std::vector<double> data(size * size);
    for (std::size_t r = 0; r < size; ++r) {
        for (std::size_t c = 0; c < size; ++c) {
            double x = static_cast<double>(c);
            double y = static_cast<double>(r);
            double dx = x - cx;
            double dy = y - cy;
            data[r * size + c] = std::exp(-(dx * dx + dy * dy) / (2.0 * sigma * sigma));
        }
    }
    return std::make_shared<Grid2D>(
        QStringLiteral("gaussian"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));
}

/// Helper: create a simple ramp grid.
std::shared_ptr<Grid2D> makeRampGrid(std::size_t size)
{
    Dimension dimX{
        QStringLiteral("x"),
        Unit::dimensionless(),
        size,
        CoordinateArray(0.0, 1.0, size),
    };
    Dimension dimY{
        QStringLiteral("y"),
        Unit::dimensionless(),
        size,
        CoordinateArray(0.0, 1.0, size),
    };
    std::vector<double> data(size * size);
    for (std::size_t r = 0; r < size; ++r) {
        for (std::size_t c = 0; c < size; ++c) {
            data[r * size + c] = static_cast<double>(r * size + c);
        }
    }
    return std::make_shared<Grid2D>(
        QStringLiteral("ramp"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));
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

} // anonymous namespace

TEST_CASE("ContourPlot: type returns Contour", "[contour_plot]")
{
    auto grid = makeGaussianGrid(11);
    ContourPlot cp(grid);
    REQUIRE(cp.type() == PlotItem::Type::Contour);
}

TEST_CASE("ContourPlot: null grid throws", "[contour_plot]")
{
    REQUIRE_THROWS_AS(ContourPlot(nullptr), std::invalid_argument);
}

TEST_CASE("ContourPlot: dataBounds matches grid coordinates", "[contour_plot]")
{
    auto grid = makeRampGrid(5);
    ContourPlot cp(grid);

    QRectF bounds = cp.dataBounds();
    REQUIRE_THAT(bounds.left(), WithinRel(0.0, 1e-9));
    REQUIRE_THAT(bounds.right(), WithinRel(4.0, 1e-9));
    REQUIRE_THAT(bounds.top(), WithinRel(0.0, 1e-9));
    REQUIRE_THAT(bounds.bottom(), WithinRel(4.0, 1e-9));
}

TEST_CASE("ContourPlot: auto-levels generates correct count", "[contour_plot]")
{
    auto grid = makeRampGrid(10);
    ContourPlot cp(grid);

    cp.setAutoLevels(5);
    REQUIRE(cp.levels().size() == 5);

    // Levels should be between min (0) and max (99), exclusive.
    for (double level : cp.levels()) {
        REQUIRE(level > 0.0);
        REQUIRE(level < 99.0);
    }
}

TEST_CASE("ContourPlot: manual levels honored", "[contour_plot]")
{
    auto grid = makeRampGrid(5);
    ContourPlot cp(grid);

    cp.setLevels({5.0, 10.0, 15.0});
    REQUIRE(cp.levels().size() == 3);
    REQUIRE_THAT(cp.levels()[0], WithinAbs(5.0, 1e-12));
    REQUIRE_THAT(cp.levels()[1], WithinAbs(10.0, 1e-12));
    REQUIRE_THAT(cp.levels()[2], WithinAbs(15.0, 1e-12));
}

TEST_CASE("ContourPlot: paint draws contour lines", "[contour_plot]")
{
    auto grid = makeGaussianGrid(21);
    ContourPlot cp(grid);
    cp.setAutoLevels(5);
    cp.setLineColor(Qt::red);
    cp.setLineWidth(2.0);

    QImage canvas(300, 300, QImage::Format_ARGB32);
    canvas.fill(Qt::white);

    CoordinateMapper mapper(0.0, 20.0, 0.0, 20.0, QRectF(0, 0, 300, 300));
    QRectF plotArea(0, 0, 300, 300);

    QPainter painter(&canvas);
    cp.paint(&painter, mapper, plotArea);
    painter.end();

    REQUIRE(hasNonWhitePixels(canvas));
}

TEST_CASE("ContourPlot: invisible does not paint", "[contour_plot]")
{
    auto grid = makeGaussianGrid(11);
    ContourPlot cp(grid);
    cp.setAutoLevels(5);
    cp.setVisible(false);

    QImage canvas(200, 200, QImage::Format_ARGB32);
    canvas.fill(Qt::white);

    CoordinateMapper mapper(0.0, 10.0, 0.0, 10.0, QRectF(0, 0, 200, 200));
    QRectF plotArea(0, 0, 200, 200);

    QPainter painter(&canvas);
    cp.paint(&painter, mapper, plotArea);
    painter.end();

    REQUIRE_FALSE(hasNonWhitePixels(canvas));
}

TEST_CASE("ContourPlot: labels can be enabled", "[contour_plot]")
{
    auto grid = makeGaussianGrid(21);
    ContourPlot cp(grid);
    cp.setAutoLevels(3);
    cp.setLabelsVisible(true);

    REQUIRE(cp.labelsVisible());

    // Paint with labels; should still produce pixels.
    QImage canvas(300, 300, QImage::Format_ARGB32);
    canvas.fill(Qt::white);

    CoordinateMapper mapper(0.0, 20.0, 0.0, 20.0, QRectF(0, 0, 300, 300));
    QRectF plotArea(0, 0, 300, 300);

    QPainter painter(&canvas);
    cp.paint(&painter, mapper, plotArea);
    painter.end();

    REQUIRE(hasNonWhitePixels(canvas));
}

TEST_CASE("ContourPlot: overlay with Heatmap", "[contour_plot]")
{
    auto grid = makeGaussianGrid(21);

    // Paint heatmap first, then contour on top.
    QImage canvas(300, 300, QImage::Format_ARGB32);
    canvas.fill(Qt::white);

    CoordinateMapper mapper(0.0, 20.0, 0.0, 20.0, QRectF(0, 0, 300, 300));
    QRectF plotArea(0, 0, 300, 300);

    QPainter painter(&canvas);

    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));
    hm.paint(&painter, mapper, plotArea);

    ContourPlot cp(grid);
    cp.setAutoLevels(5);
    cp.setLineColor(Qt::white);
    cp.setLineWidth(2.0);
    cp.paint(&painter, mapper, plotArea);

    painter.end();

    // The result should have both heatmap and contour drawn.
    REQUIRE(hasNonWhitePixels(canvas));
}

TEST_CASE("ContourPlot: setters work", "[contour_plot]")
{
    auto grid = makeGaussianGrid(11);
    ContourPlot cp(grid);

    cp.setLineColor(Qt::blue);
    REQUIRE(cp.lineColor() == QColor(Qt::blue));
    REQUIRE(cp.primaryColor() == QColor(Qt::blue));

    cp.setLineWidth(3.0);
    REQUIRE_THAT(cp.lineWidth(), WithinAbs(3.0, 1e-12));

    cp.setName("mycontour");
    REQUIRE(cp.name() == "mycontour");
}
