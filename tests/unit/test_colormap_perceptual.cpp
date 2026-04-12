#include <catch2/catch_test_macros.hpp>

#include "plot/Colormap.h"

using lumen::plot::Colormap;

TEST_CASE("Colormap perceptual uniformity: Viridis", "[colormap][perceptual]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Viridis);
    CHECK(cm.isPerceptuallyUniform());
}

TEST_CASE("Colormap perceptual uniformity: Plasma", "[colormap][perceptual]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Plasma);
    CHECK(cm.isPerceptuallyUniform());
}

TEST_CASE("Colormap perceptual uniformity: Inferno", "[colormap][perceptual]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Inferno);
    CHECK(cm.isPerceptuallyUniform());
}

TEST_CASE("Colormap perceptual uniformity: Magma", "[colormap][perceptual]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Magma);
    CHECK(cm.isPerceptuallyUniform());
}

TEST_CASE("Colormap perceptual uniformity: Cividis", "[colormap][perceptual]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Cividis);
    CHECK(cm.isPerceptuallyUniform());
}

TEST_CASE("Colormap perceptual uniformity: non-uniform map fails",
          "[colormap][perceptual]")
{
    // Deliberately create a map with very non-uniform steps:
    // huge jump in the middle, tiny steps elsewhere.
    std::vector<std::pair<double, QColor>> stops;
    stops.emplace_back(0.00, QColor(0, 0, 0));
    stops.emplace_back(0.49, QColor(1, 1, 1));
    stops.emplace_back(0.50, QColor(255, 255, 0));
    stops.emplace_back(1.00, QColor(255, 255, 1));

    const auto cm = Colormap::fromControlPoints(stops, "non-uniform");
    CHECK_FALSE(cm.isPerceptuallyUniform());
}

TEST_CASE("Colormap perceptual uniformity: result is cached",
          "[colormap][perceptual]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Viridis);
    const bool first  = cm.isPerceptuallyUniform();
    const bool second = cm.isPerceptuallyUniform();
    CHECK(first == second);
}
