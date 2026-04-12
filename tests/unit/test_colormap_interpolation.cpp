#include <catch2/catch_test_macros.hpp>

#include "plot/Colormap.h"

using lumen::plot::Colormap;

TEST_CASE("Colormap interpolation: midpoint of two stops",
          "[colormap][interpolation]")
{
    std::vector<std::pair<double, QColor>> stops;
    stops.emplace_back(0.0, QColor(0, 0, 0));
    stops.emplace_back(1.0, QColor(200, 100, 50));

    const auto cm = Colormap::fromControlPoints(stops, "lerp-test");
    const auto mid = cm.sample(0.5);

    CHECK(mid.red()   == 100);
    CHECK(mid.green() == 50);
    CHECK(mid.blue()  == 25);
}

TEST_CASE("Colormap interpolation: quarter point", "[colormap][interpolation]")
{
    std::vector<std::pair<double, QColor>> stops;
    stops.emplace_back(0.0, QColor(0, 0, 0));
    stops.emplace_back(1.0, QColor(200, 100, 0));

    const auto cm = Colormap::fromControlPoints(stops, "quarter");
    const auto q = cm.sample(0.25);

    CHECK(q.red()   == 50);
    CHECK(q.green() == 25);
    CHECK(q.blue()  == 0);
}

TEST_CASE("Colormap interpolation: multi-segment exact stops",
          "[colormap][interpolation]")
{
    std::vector<std::pair<double, QColor>> stops;
    stops.emplace_back(0.0, QColor(255, 0, 0));
    stops.emplace_back(0.5, QColor(0, 255, 0));
    stops.emplace_back(1.0, QColor(0, 0, 255));

    const auto cm = Colormap::fromControlPoints(stops, "rgb");

    // At exact stops
    const auto s0 = cm.sample(0.0);
    CHECK(s0.red()   == 255);
    CHECK(s0.green() == 0);
    CHECK(s0.blue()  == 0);

    const auto s5 = cm.sample(0.5);
    CHECK(s5.red()   == 0);
    CHECK(s5.green() == 255);
    CHECK(s5.blue()  == 0);

    const auto s1 = cm.sample(1.0);
    CHECK(s1.red()   == 0);
    CHECK(s1.green() == 0);
    CHECK(s1.blue()  == 255);
}

TEST_CASE("Colormap interpolation: between second and third stop",
          "[colormap][interpolation]")
{
    std::vector<std::pair<double, QColor>> stops;
    stops.emplace_back(0.0, QColor(255, 0, 0));
    stops.emplace_back(0.5, QColor(0, 255, 0));
    stops.emplace_back(1.0, QColor(0, 0, 255));

    const auto cm = Colormap::fromControlPoints(stops, "rgb");

    // t=0.75 is midpoint between stop2 (0,255,0) and stop3 (0,0,255)
    const auto s = cm.sample(0.75);
    CHECK(s.red()   == 0);
    CHECK(s.green() == 128);  // 255 * 0.5 rounded
    CHECK(s.blue()  == 128);  // 255 * 0.5 rounded
}
