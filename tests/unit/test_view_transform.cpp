#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "plot/ViewTransform.h"

using lumen::plot::ViewTransform;
using Catch::Matchers::WithinAbs;

TEST_CASE("ViewTransform: pan shifts range correctly",
          "[plot][view_transform]")
{
    ViewTransform vt(0.0, 100.0, 0.0, 50.0);

    vt.pan(10.0, -5.0);

    CHECK_THAT(vt.xMin(), WithinAbs(10.0, 1e-10));
    CHECK_THAT(vt.xMax(), WithinAbs(110.0, 1e-10));
    CHECK_THAT(vt.yMin(), WithinAbs(-5.0, 1e-10));
    CHECK_THAT(vt.yMax(), WithinAbs(45.0, 1e-10));
}

TEST_CASE("ViewTransform: zoom at center keeps center stationary",
          "[plot][view_transform]")
{
    ViewTransform vt(0.0, 100.0, 0.0, 100.0);

    const double centerX = 50.0;
    const double centerY = 50.0;

    vt.zoom(2.0, centerX, centerY);

    // Center should remain at 50, 50.
    const double midX = (vt.xMin() + vt.xMax()) / 2.0;
    const double midY = (vt.yMin() + vt.yMax()) / 2.0;
    CHECK_THAT(midX, WithinAbs(centerX, 1e-10));
    CHECK_THAT(midY, WithinAbs(centerY, 1e-10));

    // Range should be halved (zoom in by 2x).
    CHECK_THAT(vt.xMax() - vt.xMin(), WithinAbs(50.0, 1e-10));
    CHECK_THAT(vt.yMax() - vt.yMin(), WithinAbs(50.0, 1e-10));
}

TEST_CASE("ViewTransform: zoom at off-center keeps that point stationary",
          "[plot][view_transform]")
{
    ViewTransform vt(0.0, 100.0, 0.0, 100.0);

    const double centerX = 25.0;
    const double centerY = 75.0;

    vt.zoom(2.0, centerX, centerY);

    // The center point should remain unchanged in the view.
    // After zoom, centerX should be at the same relative position.
    // Distance from xMin to centerX should be half of original (12.5).
    CHECK_THAT(centerX - vt.xMin(), WithinAbs(12.5, 1e-10));
    CHECK_THAT(vt.xMax() - centerX, WithinAbs(37.5, 1e-10));
}

TEST_CASE("ViewTransform: zoomX only affects X range",
          "[plot][view_transform]")
{
    ViewTransform vt(0.0, 100.0, 0.0, 100.0);

    vt.zoomX(2.0, 50.0);

    // X range halved.
    CHECK_THAT(vt.xMax() - vt.xMin(), WithinAbs(50.0, 1e-10));
    // Y range unchanged.
    CHECK_THAT(vt.yMin(), WithinAbs(0.0, 1e-10));
    CHECK_THAT(vt.yMax(), WithinAbs(100.0, 1e-10));
}

TEST_CASE("ViewTransform: zoomY only affects Y range",
          "[plot][view_transform]")
{
    ViewTransform vt(0.0, 100.0, 0.0, 100.0);

    vt.zoomY(4.0, 50.0);

    // Y range quartered.
    CHECK_THAT(vt.yMax() - vt.yMin(), WithinAbs(25.0, 1e-10));
    // X range unchanged.
    CHECK_THAT(vt.xMin(), WithinAbs(0.0, 1e-10));
    CHECK_THAT(vt.xMax(), WithinAbs(100.0, 1e-10));
}

TEST_CASE("ViewTransform: reset restores original range",
          "[plot][view_transform]")
{
    ViewTransform vt(0.0, 100.0, -50.0, 50.0);

    vt.pan(30.0, -20.0);
    vt.zoom(3.0, 50.0, 25.0);

    // After multiple transforms, reset should restore original.
    vt.reset();

    CHECK_THAT(vt.xMin(), WithinAbs(0.0, 1e-10));
    CHECK_THAT(vt.xMax(), WithinAbs(100.0, 1e-10));
    CHECK_THAT(vt.yMin(), WithinAbs(-50.0, 1e-10));
    CHECK_THAT(vt.yMax(), WithinAbs(50.0, 1e-10));
}

TEST_CASE("ViewTransform: setBaseRange resets view",
          "[plot][view_transform]")
{
    ViewTransform vt(0.0, 100.0, 0.0, 100.0);

    vt.pan(10.0, 10.0);
    vt.setBaseRange(-50.0, 50.0, -25.0, 25.0);

    CHECK_THAT(vt.xMin(), WithinAbs(-50.0, 1e-10));
    CHECK_THAT(vt.xMax(), WithinAbs(50.0, 1e-10));
    CHECK_THAT(vt.yMin(), WithinAbs(-25.0, 1e-10));
    CHECK_THAT(vt.yMax(), WithinAbs(25.0, 1e-10));
}

// Phase 2.5 additional tests

TEST_CASE("ViewTransform pan shifts range exactly", "[viewtransform][p2.5]") {
    lumen::plot::ViewTransform vt(0.0, 100.0, 0.0, 100.0);
    vt.pan(10.0, 20.0);
    CHECK(vt.xMin() == Catch::Approx(10.0));
    CHECK(vt.xMax() == Catch::Approx(110.0));
    CHECK(vt.yMin() == Catch::Approx(20.0));
    CHECK(vt.yMax() == Catch::Approx(120.0));
}

TEST_CASE("ViewTransform zoom at center preserves center", "[viewtransform][p2.5]") {
    lumen::plot::ViewTransform vt(0.0, 100.0, 0.0, 100.0);
    double cx = 50.0, cy = 50.0;
    vt.zoom(2.0, cx, cy);
    double midX = (vt.xMin() + vt.xMax()) / 2.0;
    double midY = (vt.yMin() + vt.yMax()) / 2.0;
    CHECK(midX == Catch::Approx(cx).epsilon(0.01));
    CHECK(midY == Catch::Approx(cy).epsilon(0.01));
}

TEST_CASE("ViewTransform zoomX does not change Y", "[viewtransform][p2.5]") {
    lumen::plot::ViewTransform vt(0.0, 100.0, 0.0, 100.0);
    double yMinBefore = vt.yMin();
    double yMaxBefore = vt.yMax();
    vt.zoomX(2.0, 50.0);
    CHECK(vt.yMin() == yMinBefore);
    CHECK(vt.yMax() == yMaxBefore);
    CHECK(vt.xMax() - vt.xMin() < 100.0);
}
