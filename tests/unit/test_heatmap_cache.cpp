// Unit tests for Heatmap QImage caching (T8).
//
// Verifies that the cached image is reused across repeated paint() calls
// and correctly invalidated when data-affecting setters are called.

#include <catch2/catch_test_macros.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Unit.h>
#include <plot/Colormap.h>
#include <plot/CoordinateMapper.h>
#include <plot/Heatmap.h>

#include <QImage>
#include <QPainter>

#include <memory>
#include <vector>

using namespace lumen::data;
using namespace lumen::plot;

namespace {

/// Helper: create a small 4x4 Grid2D with a linear gradient.
std::shared_ptr<Grid2D> make4x4Grid()
{
    Dimension dimX{
        QStringLiteral("x"),
        Unit::dimensionless(),
        4,
        CoordinateArray(0.0, 1.0, 4),
    };
    Dimension dimY{
        QStringLiteral("y"),
        Unit::dimensionless(),
        4,
        CoordinateArray(0.0, 1.0, 4),
    };
    // 16 values: 0..15 as doubles.
    std::vector<double> data(16);
    for (int i = 0; i < 16; ++i) {
        data[static_cast<std::size_t>(i)] = static_cast<double>(i);
    }
    return std::make_shared<Grid2D>(
        QStringLiteral("test4x4"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));
}

/// Paint the heatmap onto a QImage canvas and return it.
QImage paintHeatmap(const Heatmap& hm)
{
    QImage canvas(100, 100, QImage::Format_ARGB32);
    canvas.fill(Qt::white);
    CoordinateMapper mapper(0.0, 3.0, 0.0, 3.0, QRectF(0, 0, 100, 100));
    QRectF plotArea(0, 0, 100, 100);
    QPainter painter(&canvas);
    hm.paint(&painter, mapper, plotArea);
    painter.end();
    return canvas;
}

} // anonymous namespace

TEST_CASE("Heatmap image cache: first paint rebuilds, second reuses",
          "[heatmap][cache]")
{
    auto grid = make4x4Grid();
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    // Before first paint, cache is dirty.
    REQUIRE_FALSE(hm.isImageCacheClean());

    // First paint: rebuilds the cache.
    QImage result1 = paintHeatmap(hm);
    REQUIRE(hm.isImageCacheClean());

    // Second paint: cache is still clean (reused, no rebuild).
    QImage result2 = paintHeatmap(hm);
    REQUIRE(hm.isImageCacheClean());

    // Both renders should produce identical output.
    REQUIRE(result1 == result2);
}

TEST_CASE("Heatmap image cache: setColormap invalidates cache",
          "[heatmap][cache]")
{
    auto grid = make4x4Grid();
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    // Prime the cache.
    paintHeatmap(hm);
    REQUIRE(hm.isImageCacheClean());

    // Change colormap — cache must be invalidated.
    hm.setColormap(Colormap::builtin(Colormap::Builtin::Plasma));
    REQUIRE_FALSE(hm.isImageCacheClean());

    // Paint again — rebuilds with new colormap.
    paintHeatmap(hm);
    REQUIRE(hm.isImageCacheClean());
}

TEST_CASE("Heatmap image cache: setValueRange invalidates cache",
          "[heatmap][cache]")
{
    auto grid = make4x4Grid();
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    paintHeatmap(hm);
    REQUIRE(hm.isImageCacheClean());

    hm.setValueRange(0.0, 100.0);
    REQUIRE_FALSE(hm.isImageCacheClean());

    paintHeatmap(hm);
    REQUIRE(hm.isImageCacheClean());
}

TEST_CASE("Heatmap image cache: setAutoValueRange invalidates cache",
          "[heatmap][cache]")
{
    auto grid = make4x4Grid();
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));
    hm.setValueRange(0.0, 1.0);

    paintHeatmap(hm);
    REQUIRE(hm.isImageCacheClean());

    hm.setAutoValueRange();
    REQUIRE_FALSE(hm.isImageCacheClean());
}

TEST_CASE("Heatmap image cache: setOpacity invalidates cache",
          "[heatmap][cache]")
{
    auto grid = make4x4Grid();
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    paintHeatmap(hm);
    REQUIRE(hm.isImageCacheClean());

    hm.setOpacity(0.5);
    REQUIRE_FALSE(hm.isImageCacheClean());
}

TEST_CASE("Heatmap image cache: setInterpolation does NOT invalidate cache",
          "[heatmap][cache]")
{
    auto grid = make4x4Grid();
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    paintHeatmap(hm);
    REQUIRE(hm.isImageCacheClean());

    // Interpolation only affects QPainter render hint, not the raster data.
    hm.setInterpolation(Heatmap::Interpolation::Bilinear);
    REQUIRE(hm.isImageCacheClean());
}

TEST_CASE("Heatmap image cache: setName and setVisible do NOT invalidate cache",
          "[heatmap][cache]")
{
    auto grid = make4x4Grid();
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    paintHeatmap(hm);
    REQUIRE(hm.isImageCacheClean());

    hm.setName(QStringLiteral("renamed"));
    REQUIRE(hm.isImageCacheClean());

    hm.setVisible(false);
    REQUIRE(hm.isImageCacheClean());
}

TEST_CASE("Heatmap image cache: different colormaps produce different images",
          "[heatmap][cache]")
{
    auto grid = make4x4Grid();

    Heatmap hmV(grid, Colormap::builtin(Colormap::Builtin::Viridis));
    Heatmap hmP(grid, Colormap::builtin(Colormap::Builtin::Plasma));

    QImage imgV = paintHeatmap(hmV);
    QImage imgP = paintHeatmap(hmP);

    // The two colormaps should produce visually different images.
    REQUIRE(imgV != imgP);
}
