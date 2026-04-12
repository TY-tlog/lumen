#include <catch2/catch_test_macros.hpp>

#include "plot/Colormap.h"

#include <QJsonArray>
#include <QJsonObject>

using lumen::plot::Colormap;

TEST_CASE("Colormap JSON roundtrip: built-in Viridis", "[colormap][json]")
{
    const auto original = Colormap::builtin(Colormap::Builtin::Viridis);
    const QJsonObject json = original.toJson();
    const auto restored = Colormap::fromJson(json);

    CHECK(restored.name() == original.name());
    REQUIRE(restored.stops().size() == original.stops().size());

    // Check samples at several positions
    for (double t : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        const auto c1 = original.sample(t);
        const auto c2 = restored.sample(t);
        INFO("t=" << t);
        CHECK(c1.red()   == c2.red());
        CHECK(c1.green() == c2.green());
        CHECK(c1.blue()  == c2.blue());
    }
}

TEST_CASE("Colormap JSON roundtrip: all built-ins preserve endpoints",
          "[colormap][json]")
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
        const auto orig = Colormap::builtin(id);
        const auto rest = Colormap::fromJson(orig.toJson());
        INFO("Colormap: " << orig.name().toStdString());
        CHECK(rest.sample(0.0) == orig.sample(0.0));
        CHECK(rest.sample(1.0) == orig.sample(1.0));
    }
}

TEST_CASE("Colormap JSON roundtrip: custom colormap", "[colormap][json]")
{
    std::vector<std::pair<double, QColor>> stops;
    stops.emplace_back(0.0, QColor(10, 20, 30));
    stops.emplace_back(0.5, QColor(100, 150, 200));
    stops.emplace_back(1.0, QColor(250, 240, 230));

    const auto original = Colormap::fromControlPoints(stops, "custom");
    const auto restored = Colormap::fromJson(original.toJson());

    CHECK(restored.name() == "custom");
    for (double t : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        INFO("t=" << t);
        CHECK(original.sample(t) == restored.sample(t));
    }
}

TEST_CASE("Colormap JSON: toJson structure", "[colormap][json]")
{
    const auto cm = Colormap::builtin(Colormap::Builtin::Gray);
    const auto json = cm.toJson();

    CHECK(json.contains("name"));
    CHECK(json.contains("stops"));
    CHECK(json.value("name").toString() == "Gray");

    const auto stops = json.value("stops").toArray();
    CHECK(stops.size() == 3); // Gray has 3 stops
    CHECK(stops[0].toArray().size() == 4); // [t, r, g, b]
}
