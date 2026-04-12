#include <catch2/catch_test_macros.hpp>

#include "plot/Colormap.h"

#include <stdexcept>

using lumen::plot::Colormap;

TEST_CASE("Colormap edge: t=0 returns first stop", "[colormap][edge]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Viridis);
    const auto c = cm.sample(0.0);
    CHECK(c == cm.stops().front().second);
}

TEST_CASE("Colormap edge: t=1 returns last stop", "[colormap][edge]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Viridis);
    const auto c = cm.sample(1.0);
    CHECK(c == cm.stops().back().second);
}

TEST_CASE("Colormap edge: t<0 clamped to t=0", "[colormap][edge]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Plasma);
    CHECK(cm.sample(-0.5) == cm.sample(0.0));
    CHECK(cm.sample(-100.0) == cm.sample(0.0));
}

TEST_CASE("Colormap edge: t>1 clamped to t=1", "[colormap][edge]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Plasma);
    CHECK(cm.sample(1.5) == cm.sample(1.0));
    CHECK(cm.sample(999.0) == cm.sample(1.0));
}

TEST_CASE("Colormap edge: fewer than 2 stops throws", "[colormap][edge]")
{
    std::vector<std::pair<double, QColor>> stops;
    stops.emplace_back(0.5, QColor(128, 128, 128));

    CHECK_THROWS_AS(Colormap::fromControlPoints(stops, "one-stop"),
                    std::invalid_argument);
}

TEST_CASE("Colormap edge: unsorted stops are sorted automatically",
          "[colormap][edge]")
{
    std::vector<std::pair<double, QColor>> stops;
    stops.emplace_back(1.0, QColor(255, 255, 255));
    stops.emplace_back(0.0, QColor(0, 0, 0));

    const auto cm = Colormap::fromControlPoints(stops, "reversed");

    // After sorting: t=0 -> black, t=1 -> white
    CHECK(cm.sample(0.0) == QColor(0, 0, 0));
    CHECK(cm.sample(1.0) == QColor(255, 255, 255));
}
