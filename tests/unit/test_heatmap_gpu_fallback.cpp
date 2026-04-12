// Unit tests for Heatmap GPU fallback (T9).
//
// In QT_QPA_PLATFORM=offscreen (test environment) there is no GL context.
// Verify that the CPU path works correctly regardless of activeRenderPath().

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
    // Fill with a simple gradient so the image is non-trivial.
    std::vector<double> data(size * size);
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<double>(i) / static_cast<double>(data.size());
    }
    return std::make_shared<Grid2D>(
        QStringLiteral("square"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));
}

} // anonymous namespace

TEST_CASE("Heatmap GPU fallback: large grid still renders via CPU paint()",
          "[heatmap][gpu][fallback]")
{
    // 1025x1025 => ~1.05M cells, just above the 1024*1024 threshold
    // so activeRenderPath() == GPU. Keeping it small so ASAN tests
    // finish quickly.
    auto grid = makeSquareGrid(1025);
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    REQUIRE(hm.activeRenderPath() == Heatmap::RenderPath::GPU);

    // Paint should succeed even without a GL context.
    // CoordinateArray(0.0, 1.0, 1025) => coords 0,1,...,1024.
    QImage canvas(200, 200, QImage::Format_ARGB32);
    canvas.fill(Qt::white);
    CoordinateMapper mapper(0.0, 1024.0, 0.0, 1024.0, QRectF(0, 0, 200, 200));
    QRectF plotArea(0, 0, 200, 200);

    QPainter painter(&canvas);
    hm.paint(&painter, mapper, plotArea);
    painter.end();

    // The rendered image should have color variation (not just white).
    QRgb first = canvas.pixel(0, 0);
    bool hasVariation = false;
    for (int y = 0; y < canvas.height() && !hasVariation; y += 10) {
        for (int x = 0; x < canvas.width() && !hasVariation; x += 10) {
            if (canvas.pixel(x, y) != first) {
                hasVariation = true;
            }
        }
    }
    REQUIRE(hasVariation);
}

TEST_CASE("Heatmap GPU fallback: renderGpu with nullptr is a no-op",
          "[heatmap][gpu][fallback]")
{
    auto grid = makeSquareGrid(4);
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    CoordinateMapper mapper(0.0, 1.0, 0.0, 1.0, QRectF(0, 0, 100, 100));
    QRectF plotArea(0, 0, 100, 100);

    // Calling renderGpu with nullptr should not crash.
    hm.renderGpu(nullptr, mapper, plotArea);
}

TEST_CASE("Heatmap GPU fallback: cache works for large grids too",
          "[heatmap][gpu][fallback][cache]")
{
    auto grid = makeSquareGrid(1025);
    Heatmap hm(grid, Colormap::builtin(Colormap::Builtin::Viridis));

    REQUIRE_FALSE(hm.isImageCacheClean());

    // First paint builds cache.
    QImage canvas(100, 100, QImage::Format_ARGB32);
    canvas.fill(Qt::white);
    CoordinateMapper mapper(0.0, 1024.0, 0.0, 1024.0, QRectF(0, 0, 100, 100));
    QRectF plotArea(0, 0, 100, 100);

    QPainter painter(&canvas);
    hm.paint(&painter, mapper, plotArea);
    painter.end();
    REQUIRE(hm.isImageCacheClean());

    // Second paint reuses cache.
    QImage canvas2(100, 100, QImage::Format_ARGB32);
    canvas2.fill(Qt::white);
    QPainter painter2(&canvas2);
    hm.paint(&painter2, mapper, plotArea);
    painter2.end();
    REQUIRE(hm.isImageCacheClean());
}
