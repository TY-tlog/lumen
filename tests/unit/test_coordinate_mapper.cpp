#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "plot/CoordinateMapper.h"

using lumen::plot::CoordinateMapper;
using Catch::Matchers::WithinAbs;

TEST_CASE("CoordinateMapper: round-trip dataToPixel -> pixelToData",
          "[plot][coordinate_mapper]")
{
    const CoordinateMapper mapper(0.0, 100.0, -50.0, 50.0,
                                  QRectF(50.0, 20.0, 400.0, 300.0));

    const double testX = 37.5;
    const double testY = 12.3;

    const QPointF pixel = mapper.dataToPixel(testX, testY);
    const auto [roundX, roundY] = mapper.pixelToData(pixel);

    CHECK_THAT(roundX, WithinAbs(testX, 1e-10));
    CHECK_THAT(roundY, WithinAbs(testY, 1e-10));
}

TEST_CASE("CoordinateMapper: Y inversion — higher data Y gives lower pixel Y",
          "[plot][coordinate_mapper]")
{
    const CoordinateMapper mapper(0.0, 100.0, 0.0, 100.0,
                                  QRectF(0.0, 0.0, 400.0, 300.0));

    const QPointF low = mapper.dataToPixel(50.0, 10.0);
    const QPointF high = mapper.dataToPixel(50.0, 90.0);

    // Higher Y data -> lower pixel Y value.
    CHECK(high.y() < low.y());
}

TEST_CASE("CoordinateMapper: edge values map to rect edges",
          "[plot][coordinate_mapper]")
{
    const QRectF rect(50.0, 20.0, 400.0, 300.0);
    const CoordinateMapper mapper(0.0, 100.0, 0.0, 100.0, rect);

    // Data (0, 0) -> bottom-left of pixel rect (Y inverted: yMin maps to bottom).
    const QPointF bottomLeft = mapper.dataToPixel(0.0, 0.0);
    CHECK_THAT(bottomLeft.x(), WithinAbs(rect.left(), 1e-10));
    CHECK_THAT(bottomLeft.y(), WithinAbs(rect.bottom(), 1e-10));

    // Data (100, 100) -> top-right of pixel rect.
    const QPointF topRight = mapper.dataToPixel(100.0, 100.0);
    CHECK_THAT(topRight.x(), WithinAbs(rect.right(), 1e-10));
    CHECK_THAT(topRight.y(), WithinAbs(rect.top(), 1e-10));
}

TEST_CASE("CoordinateMapper: multiple round-trips at different points",
          "[plot][coordinate_mapper]")
{
    const CoordinateMapper mapper(-200.0, 500.0, -80.0, 40.0,
                                  QRectF(60.0, 10.0, 600.0, 400.0));

    const std::vector<std::pair<double, double>> testPoints = {
        {-200.0, -80.0},  // min corner
        {500.0, 40.0},    // max corner
        {0.0, 0.0},       // origin
        {150.0, -20.0},   // arbitrary
        {-100.0, 35.0},   // negative X
    };

    for (const auto& [x, y] : testPoints) {
        const QPointF pixel = mapper.dataToPixel(x, y);
        const auto [rx, ry] = mapper.pixelToData(pixel);
        CHECK_THAT(rx, WithinAbs(x, 1e-10));
        CHECK_THAT(ry, WithinAbs(y, 1e-10));
    }
}

TEST_CASE("CoordinateMapper: setDataRange updates mapping",
          "[plot][coordinate_mapper]")
{
    CoordinateMapper mapper(0.0, 100.0, 0.0, 100.0,
                            QRectF(0.0, 0.0, 100.0, 100.0));

    // Before: (50, 50) maps to center.
    QPointF p1 = mapper.dataToPixel(50.0, 50.0);
    CHECK_THAT(p1.x(), WithinAbs(50.0, 1e-10));
    CHECK_THAT(p1.y(), WithinAbs(50.0, 1e-10));

    // Change data range: now 0-200 on both axes.
    mapper.setDataRange(0.0, 200.0, 0.0, 200.0);

    // (50, 50) should now map to 1/4 of pixel rect.
    QPointF p2 = mapper.dataToPixel(50.0, 50.0);
    CHECK_THAT(p2.x(), WithinAbs(25.0, 1e-10));
    CHECK_THAT(p2.y(), WithinAbs(75.0, 1e-10)); // Y inverted
}

TEST_CASE("CoordinateMapper: setPixelRect updates mapping",
          "[plot][coordinate_mapper]")
{
    CoordinateMapper mapper(0.0, 100.0, 0.0, 100.0,
                            QRectF(0.0, 0.0, 100.0, 100.0));

    mapper.setPixelRect(QRectF(0.0, 0.0, 200.0, 200.0));

    // (50, 50) should map to center of new rect.
    const QPointF p = mapper.dataToPixel(50.0, 50.0);
    CHECK_THAT(p.x(), WithinAbs(100.0, 1e-10));
    CHECK_THAT(p.y(), WithinAbs(100.0, 1e-10));
}
