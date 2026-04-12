#include <catch2/catch_test_macros.hpp>

#include "plot/Colormap.h"

using lumen::plot::Colormap;

TEST_CASE("Colormap CVD safety: Cividis passes", "[colormap][cvd]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Cividis);
    CHECK(cm.isColorblindSafe());
}

TEST_CASE("Colormap CVD safety: Gray passes (monotone luminance)",
          "[colormap][cvd]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Gray);
    CHECK(cm.isColorblindSafe());
}

TEST_CASE("Colormap CVD safety: result is cached", "[colormap][cvd]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Cividis);
    const bool first  = cm.isColorblindSafe();
    const bool second = cm.isColorblindSafe();
    CHECK(first == second);
}

TEST_CASE("Colormap CVD safety: low-contrast map fails", "[colormap][cvd]")
{
    // Two very similar red/green stops: CVD collapses them.
    std::vector<std::pair<double, QColor>> stops;
    stops.emplace_back(0.0, QColor(180, 60, 60));
    stops.emplace_back(1.0, QColor(60, 180, 60));

    const auto cm = Colormap::fromControlPoints(stops, "rg-unsafe");
    CHECK_FALSE(cm.isColorblindSafe());
}
