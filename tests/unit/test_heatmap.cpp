// Unit tests for Heatmap PlotItem (T6+T7).

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Unit.h>
#include <plot/Colormap.h>
#include <plot/CoordinateMapper.h>
#include <plot/Heatmap.h>

#include <QImage>
#include <QPainter>

#include <cmath>
#include <memory>
#include <vector>

using namespace lumen::data;
using namespace lumen::plot;
using Catch::Matchers::WithinRel;

namespace {

/// Helper: create a small 3x2 Grid2D with known values.
/// dimX has 3 columns spanning x=[0, 2], dimY has 2 rows spanning y=[0, 1].
/// Values: row0 = {1,2,3}, row1 = {4,5,6}.
std::shared_ptr<Grid2D> makeSmallGrid()
{
    Dimension dimX{
        QStringLiteral("x"),
        Unit::parse(QStringLiteral("m")),
        3,
        CoordinateArray(0.0, 1.0, 3),
    };
    Dimension dimY{
        QStringLiteral("y"),
        Unit::parse(QStringLiteral("m")),
        2,
        CoordinateArray(0.0, 1.0, 2),
    };
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    return std::make_shared<Grid2D>(
        QStringLiteral("test"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));
}

/// Helper: create a square grid of given size.
std::shared_ptr<Grid2D> makeSquareGrid(std::size_t size)
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
    std::vector<double> data(size * size, 0.5);
    return std::make_shared<Grid2D>(
        QStringLiteral("square"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));
}

} // anonymous namespace

TEST_CASE("Heatmap type() returns Type::Heatmap", "[heatmap]")
{
    auto grid = makeSmallGrid();
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));
    REQUIRE(hm.type() == PlotItem::Type::Heatmap);
}

TEST_CASE("Heatmap dataBounds() matches Grid2D coordinate ranges", "[heatmap]")
{
    auto grid = makeSmallGrid();
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    QRectF bounds = hm.dataBounds();
    // x range: [0, 2], y range: [0, 1]
    REQUIRE_THAT(bounds.left(), WithinRel(0.0, 1e-9));
    REQUIRE_THAT(bounds.right(), WithinRel(2.0, 1e-9));
    REQUIRE_THAT(bounds.top(), WithinRel(0.0, 1e-9));
    REQUIRE_THAT(bounds.bottom(), WithinRel(1.0, 1e-9));
}

TEST_CASE("Heatmap CPU paint produces non-empty image with varied colors", "[heatmap]")
{
    auto grid = makeSmallGrid();
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    // Render onto a QImage via QPainter
    QImage canvas(200, 200, QImage::Format_ARGB32);
    canvas.fill(Qt::white);

    CoordinateMapper mapper(0.0, 2.0, 0.0, 1.0, QRectF(0, 0, 200, 200));
    QRectF plotArea(0, 0, 200, 200);

    QPainter painter(&canvas);
    hm.paint(&painter, mapper, plotArea);
    painter.end();

    // Check that the image is not entirely one color (the heatmap was drawn)
    QRgb firstPixel = canvas.pixel(0, 0);
    bool hasVariation = false;
    for (int y = 0; y < canvas.height() && !hasVariation; ++y) {
        for (int x = 0; x < canvas.width() && !hasVariation; ++x) {
            if (canvas.pixel(x, y) != firstPixel) {
                hasVariation = true;
            }
        }
    }
    REQUIRE(hasVariation);
}

TEST_CASE("Heatmap manual value range honored", "[heatmap]")
{
    // Create a grid with uniform value 0.5
    Dimension dimX{
        QStringLiteral("x"), Unit::dimensionless(), 2,
        CoordinateArray(0.0, 1.0, 2),
    };
    Dimension dimY{
        QStringLiteral("y"), Unit::dimensionless(), 2,
        CoordinateArray(0.0, 1.0, 2),
    };
    std::vector<double> data = {0.5, 0.5, 0.5, 0.5};
    auto grid = std::make_shared<Grid2D>(
        QStringLiteral("uniform"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));

    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    // With manual range [0, 1], value 0.5 maps to t=0.5
    hm.setValueRange(0.0, 1.0);
    QColor midColor = Colormap::builtin(Colormap::Builtin::Viridis).sample(0.5);

    QImage canvas(100, 100, QImage::Format_ARGB32);
    canvas.fill(Qt::black);
    CoordinateMapper mapper(0.0, 1.0, 0.0, 1.0, QRectF(0, 0, 100, 100));
    QRectF plotArea(0, 0, 100, 100);
    QPainter painter(&canvas);
    hm.paint(&painter, mapper, plotArea);
    painter.end();

    // Sample a pixel from the center — should match colormap at t=0.5
    QColor centerColor = canvas.pixelColor(50, 50);
    REQUIRE(std::abs(centerColor.red() - midColor.red()) <= 2);
    REQUIRE(std::abs(centerColor.green() - midColor.green()) <= 2);
    REQUIRE(std::abs(centerColor.blue() - midColor.blue()) <= 2);

    // Now with manual range [0, 2], value 0.5 maps to t=0.25
    hm.setValueRange(0.0, 2.0);
    QColor quarterColor = Colormap::builtin(Colormap::Builtin::Viridis).sample(0.25);

    QImage canvas2(100, 100, QImage::Format_ARGB32);
    canvas2.fill(Qt::black);
    QPainter painter2(&canvas2);
    hm.paint(&painter2, mapper, plotArea);
    painter2.end();

    QColor centerColor2 = canvas2.pixelColor(50, 50);
    REQUIRE(std::abs(centerColor2.red() - quarterColor.red()) <= 2);
    REQUIRE(std::abs(centerColor2.green() - quarterColor.green()) <= 2);
    REQUIRE(std::abs(centerColor2.blue() - quarterColor.blue()) <= 2);
}

TEST_CASE("Heatmap auto value range computes from data", "[heatmap]")
{
    auto grid = makeSmallGrid();
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    // Auto range is the default. Min value = 1, max value = 6.
    // The minimum value (1.0) should map to t=0 (start of colormap)
    // The maximum value (6.0) should map to t=1 (end of colormap)
    QColor startColor = Colormap::builtin(Colormap::Builtin::Viridis).sample(0.0);

    QImage canvas(300, 200, QImage::Format_ARGB32);
    canvas.fill(Qt::black);
    CoordinateMapper mapper(0.0, 2.0, 0.0, 1.0, QRectF(0, 0, 300, 200));
    QRectF plotArea(0, 0, 300, 200);
    QPainter painter(&canvas);
    hm.paint(&painter, mapper, plotArea);
    painter.end();

    // Bottom-left corner should be near value 1 (min) -> startColor
    // (grid y=0 maps to bottom of image; x=0 maps to left)
    QColor bottomLeft = canvas.pixelColor(10, 190);
    REQUIRE(std::abs(bottomLeft.red() - startColor.red()) <= 3);
    REQUIRE(std::abs(bottomLeft.green() - startColor.green()) <= 3);
    REQUIRE(std::abs(bottomLeft.blue() - startColor.blue()) <= 3);
}

TEST_CASE("Heatmap opacity < 1 produces semi-transparent pixels", "[heatmap]")
{
    Dimension dimX{
        QStringLiteral("x"), Unit::dimensionless(), 2,
        CoordinateArray(0.0, 1.0, 2),
    };
    Dimension dimY{
        QStringLiteral("y"), Unit::dimensionless(), 2,
        CoordinateArray(0.0, 1.0, 2),
    };
    std::vector<double> data = {0.5, 0.5, 0.5, 0.5};
    auto grid = std::make_shared<Grid2D>(
        QStringLiteral("semi"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));

    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));
    hm.setOpacity(0.5);
    hm.setValueRange(0.0, 1.0);

    REQUIRE(hm.opacity() == 0.5);

    // Render with full opacity first
    Heatmap hmFull(hm.grid(), Colormap::builtin(Colormap::Builtin::Viridis));
    hmFull.setValueRange(0.0, 1.0);

    QImage canvasFull(100, 100, QImage::Format_ARGB32);
    canvasFull.fill(qRgba(255, 255, 255, 255));
    CoordinateMapper mapper(0.0, 1.0, 0.0, 1.0, QRectF(0, 0, 100, 100));
    QRectF plotArea(0, 0, 100, 100);

    QPainter painterFull(&canvasFull);
    hmFull.paint(&painterFull, mapper, plotArea);
    painterFull.end();

    // Render with half opacity
    QImage canvasHalf(100, 100, QImage::Format_ARGB32);
    canvasHalf.fill(qRgba(255, 255, 255, 255));

    QPainter painterHalf(&canvasHalf);
    hm.paint(&painterHalf, mapper, plotArea);
    painterHalf.end();

    // Semi-transparent heatmap over white should differ from full-opacity
    QColor fullColor = canvasFull.pixelColor(50, 50);
    QColor halfColor = canvasHalf.pixelColor(50, 50);

    // The half-opacity color should be blended toward white compared to full
    // (unless the colormap color at t=0.5 happens to be white, which it won't be for Viridis)
    bool colorsAreDistinct = (std::abs(fullColor.red() - halfColor.red()) > 5
                              || std::abs(fullColor.green() - halfColor.green()) > 5
                              || std::abs(fullColor.blue() - halfColor.blue()) > 5);
    REQUIRE(colorsAreDistinct);
}

TEST_CASE("Heatmap activeRenderPath: 256x256 -> CPU, 2048x2048 -> GPU", "[heatmap]")
{
    auto smallGrid = makeSquareGrid(256);
    Heatmap hmSmall(smallGrid, Colormap::builtin(Colormap::Builtin::Viridis));
    REQUIRE(hmSmall.activeRenderPath() == Heatmap::RenderPath::CPU);

    auto largeGrid = makeSquareGrid(2048);
    Heatmap hmLarge(largeGrid, Colormap::builtin(Colormap::Builtin::Viridis));
    REQUIRE(hmLarge.activeRenderPath() == Heatmap::RenderPath::GPU);
}
