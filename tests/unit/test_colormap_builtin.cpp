#include <catch2/catch_test_macros.hpp>

#include "plot/Colormap.h"

using lumen::plot::Colormap;

namespace {

/// Check that sampling at t=0 and t=1 returns exactly the first/last stop.
void checkEndpoints(Colormap::Builtin id, const QColor& expectedStart,
                    const QColor& expectedEnd)
{
    const auto cm = Colormap::builtin(id);
    const auto s0 = cm.sample(0.0);
    const auto s1 = cm.sample(1.0);

    INFO("Colormap: " << cm.name().toStdString());
    CHECK(s0.red()   == expectedStart.red());
    CHECK(s0.green() == expectedStart.green());
    CHECK(s0.blue()  == expectedStart.blue());
    CHECK(s1.red()   == expectedEnd.red());
    CHECK(s1.green() == expectedEnd.green());
    CHECK(s1.blue()  == expectedEnd.blue());
}

/// Check midpoint sample is within a reasonable tolerance of an expected color.
void checkMidpoint(Colormap::Builtin id, const QColor& expected, int tol = 30)
{
    const auto cm = Colormap::builtin(id);
    const auto mid = cm.sample(0.5);

    INFO("Colormap: " << cm.name().toStdString());
    CHECK(std::abs(mid.red()   - expected.red())   <= tol);
    CHECK(std::abs(mid.green() - expected.green()) <= tol);
    CHECK(std::abs(mid.blue()  - expected.blue())  <= tol);
}

} // namespace

TEST_CASE("Colormap builtin: Viridis endpoints and midpoint", "[colormap][builtin]")
{
    checkEndpoints(Colormap::Builtin::Viridis,
                   QColor(68, 1, 84), QColor(253, 231, 37));
    // Mid should be teal-ish
    checkMidpoint(Colormap::Builtin::Viridis, QColor(34, 140, 141), 40);
}

TEST_CASE("Colormap builtin: Plasma endpoints and midpoint", "[colormap][builtin]")
{
    checkEndpoints(Colormap::Builtin::Plasma,
                   QColor(13, 8, 135), QColor(252, 255, 60));
    checkMidpoint(Colormap::Builtin::Plasma, QColor(207, 79, 108), 40);
}

TEST_CASE("Colormap builtin: Inferno endpoints and midpoint", "[colormap][builtin]")
{
    checkEndpoints(Colormap::Builtin::Inferno,
                   QColor(0, 0, 4), QColor(252, 255, 164));
    checkMidpoint(Colormap::Builtin::Inferno, QColor(168, 47, 92), 40);
}

TEST_CASE("Colormap builtin: Magma endpoints and midpoint", "[colormap][builtin]")
{
    checkEndpoints(Colormap::Builtin::Magma,
                   QColor(0, 0, 4), QColor(252, 253, 191));
    checkMidpoint(Colormap::Builtin::Magma, QColor(162, 44, 122), 40);
}

TEST_CASE("Colormap builtin: Turbo endpoints and midpoint", "[colormap][builtin]")
{
    checkEndpoints(Colormap::Builtin::Turbo,
                   QColor(48, 18, 59), QColor(122, 4, 3));
    checkMidpoint(Colormap::Builtin::Turbo, QColor(96, 245, 88), 40);
}

TEST_CASE("Colormap builtin: Cividis endpoints and midpoint", "[colormap][builtin]")
{
    checkEndpoints(Colormap::Builtin::Cividis,
                   QColor(0, 32, 76), QColor(253, 231, 37));
    checkMidpoint(Colormap::Builtin::Cividis, QColor(122, 127, 115), 40);
}

TEST_CASE("Colormap builtin: Gray endpoints and midpoint", "[colormap][builtin]")
{
    checkEndpoints(Colormap::Builtin::Gray,
                   QColor(0, 0, 0), QColor(255, 255, 255));
    checkMidpoint(Colormap::Builtin::Gray, QColor(128, 128, 128), 2);
}

TEST_CASE("Colormap builtin: Hot endpoints and midpoint", "[colormap][builtin]")
{
    checkEndpoints(Colormap::Builtin::Hot,
                   QColor(0, 0, 0), QColor(255, 255, 255));
    checkMidpoint(Colormap::Builtin::Hot, QColor(255, 85, 0), 5);
}

TEST_CASE("Colormap builtin: Cool endpoints and midpoint", "[colormap][builtin]")
{
    checkEndpoints(Colormap::Builtin::Cool,
                   QColor(0, 255, 255), QColor(255, 0, 255));
    checkMidpoint(Colormap::Builtin::Cool, QColor(128, 128, 255), 2);
}

TEST_CASE("Colormap builtin: RedBlue endpoints and midpoint", "[colormap][builtin]")
{
    checkEndpoints(Colormap::Builtin::RedBlue,
                   QColor(59, 76, 192), QColor(180, 4, 38));
    // Midpoint should be near-white
    checkMidpoint(Colormap::Builtin::RedBlue, QColor(245, 245, 245), 5);
}

TEST_CASE("Colormap builtin: BrownTeal endpoints and midpoint", "[colormap][builtin]")
{
    checkEndpoints(Colormap::Builtin::BrownTeal,
                   QColor(140, 81, 10), QColor(1, 102, 94));
    // Midpoint should be near-white
    checkMidpoint(Colormap::Builtin::BrownTeal, QColor(245, 245, 245), 5);
}

TEST_CASE("Colormap builtin: all have >=2 stops", "[colormap][builtin]")
{
    const auto ids = {
        Colormap::Builtin::Viridis, Colormap::Builtin::Plasma,
        Colormap::Builtin::Inferno, Colormap::Builtin::Magma,
        Colormap::Builtin::Turbo,   Colormap::Builtin::Cividis,
        Colormap::Builtin::Gray,    Colormap::Builtin::Hot,
        Colormap::Builtin::Cool,    Colormap::Builtin::RedBlue,
        Colormap::Builtin::BrownTeal
    };
    for (auto id : ids) {
        const auto cm = Colormap::builtin(id);
        INFO("Colormap: " << cm.name().toStdString());
        CHECK(cm.stops().size() >= 2);
    }
}
