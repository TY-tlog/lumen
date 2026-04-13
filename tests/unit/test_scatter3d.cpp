// Unit tests for Scatter3D.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <plot3d/Ray.h>
#include <plot3d/Scatter3D.h>

#include <memory>
#include <vector>

using namespace lumen::plot3d;
using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using Catch::Matchers::WithinAbs;

namespace {

struct Scatter3DFixture {
    std::shared_ptr<Rank1Dataset> xDs = std::make_shared<Rank1Dataset>(
        "x", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0});
    std::shared_ptr<Rank1Dataset> yDs = std::make_shared<Rank1Dataset>(
        "y", Unit::dimensionless(), std::vector<double>{4.0, 5.0, 6.0});
    std::shared_ptr<Rank1Dataset> zDs = std::make_shared<Rank1Dataset>(
        "z", Unit::dimensionless(), std::vector<double>{7.0, 8.0, 9.0});

    std::unique_ptr<Scatter3D> scatter = std::make_unique<Scatter3D>(
        xDs, yDs, zDs, Qt::blue, "test_scatter");
};

}  // namespace

TEST_CASE("Scatter3D: type is Scatter3D", "[scatter3d]") {
    Scatter3DFixture fix;
    CHECK(fix.scatter->type() == PlotItem3D::Type::Scatter3D);
}

TEST_CASE("Scatter3D: name and visibility", "[scatter3d]") {
    Scatter3DFixture fix;
    CHECK(fix.scatter->name() == QStringLiteral("test_scatter"));
    CHECK(fix.scatter->isVisible());

    fix.scatter->setVisible(false);
    CHECK_FALSE(fix.scatter->isVisible());

    fix.scatter->setName("renamed");
    CHECK(fix.scatter->name() == QStringLiteral("renamed"));
}

TEST_CASE("Scatter3D: dataBounds covers all points", "[scatter3d]") {
    Scatter3DFixture fix;
    BoundingBox3D bounds = fix.scatter->dataBounds();

    CHECK(bounds.isValid());
    CHECK_THAT(static_cast<double>(bounds.min.x()), WithinAbs(1.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.min.y()), WithinAbs(4.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.min.z()), WithinAbs(7.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.max.x()), WithinAbs(3.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.max.y()), WithinAbs(6.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.max.z()), WithinAbs(9.0, 1e-5));
}

TEST_CASE("Scatter3D: dataBounds empty with no data", "[scatter3d]") {
    auto xDs = std::make_shared<Rank1Dataset>(
        "x", Unit::dimensionless(), std::vector<double>{});
    auto yDs = std::make_shared<Rank1Dataset>(
        "y", Unit::dimensionless(), std::vector<double>{});
    auto zDs = std::make_shared<Rank1Dataset>(
        "z", Unit::dimensionless(), std::vector<double>{});

    Scatter3D scatter(xDs, yDs, zDs);
    BoundingBox3D bounds = scatter.dataBounds();
    // Empty bounds: min == max == (0,0,0), so size is zero.
    bool hasExtent = bounds.isValid() && (bounds.size().length() > 0.0f);
    CHECK_FALSE(hasExtent);
}

TEST_CASE("Scatter3D: hitTestRay detects point", "[scatter3d]") {
    // Place a single point at (0, 0, 0) with a generous marker size.
    auto xDs = std::make_shared<Rank1Dataset>(
        "x", Unit::dimensionless(), std::vector<double>{0.0});
    auto yDs = std::make_shared<Rank1Dataset>(
        "y", Unit::dimensionless(), std::vector<double>{0.0});
    auto zDs = std::make_shared<Rank1Dataset>(
        "z", Unit::dimensionless(), std::vector<double>{0.0});

    Scatter3D scatter(xDs, yDs, zDs, Qt::red, "single");
    scatter.setMarkerSize(0.5f);

    // Ray traveling along -Z axis, passing through origin.
    Ray ray{QVector3D(0, 0, 5), QVector3D(0, 0, -1)};

    auto hit = scatter.hitTestRay(ray, 100.0);
    REQUIRE(hit.has_value());
    CHECK_THAT(hit->distance, WithinAbs(4.5, 0.1));  // ~5.0 - 0.5 radius
}

TEST_CASE("Scatter3D: hitTestRay misses distant point", "[scatter3d]") {
    auto xDs = std::make_shared<Rank1Dataset>(
        "x", Unit::dimensionless(), std::vector<double>{10.0});
    auto yDs = std::make_shared<Rank1Dataset>(
        "y", Unit::dimensionless(), std::vector<double>{10.0});
    auto zDs = std::make_shared<Rank1Dataset>(
        "z", Unit::dimensionless(), std::vector<double>{10.0});

    Scatter3D scatter(xDs, yDs, zDs, Qt::red, "far");
    scatter.setMarkerSize(0.05f);

    // Ray along Z axis from origin - should miss point at (10,10,10).
    Ray ray{QVector3D(0, 0, 0), QVector3D(0, 0, -1)};

    auto hit = scatter.hitTestRay(ray, 100.0);
    CHECK_FALSE(hit.has_value());
}

TEST_CASE("Scatter3D: hitTestRay finds nearest of multiple points", "[scatter3d]") {
    auto xDs = std::make_shared<Rank1Dataset>(
        "x", Unit::dimensionless(), std::vector<double>{0.0, 0.0, 0.0});
    auto yDs = std::make_shared<Rank1Dataset>(
        "y", Unit::dimensionless(), std::vector<double>{0.0, 0.0, 0.0});
    auto zDs = std::make_shared<Rank1Dataset>(
        "z", Unit::dimensionless(), std::vector<double>{2.0, 5.0, 8.0});

    Scatter3D scatter(xDs, yDs, zDs, Qt::green, "multi");
    scatter.setMarkerSize(0.5f);

    // Ray from z=20 toward -Z.
    Ray ray{QVector3D(0, 0, 20), QVector3D(0, 0, -1)};

    auto hit = scatter.hitTestRay(ray, 100.0);
    REQUIRE(hit.has_value());
    // Should hit the point at z=8 first (nearest to ray origin at z=20).
    CHECK_THAT(hit->worldPoint.z(), WithinAbs(8.5, 0.1));
}

TEST_CASE("Scatter3D: marker properties", "[scatter3d]") {
    Scatter3DFixture fix;

    CHECK(fix.scatter->markerShape() == Scatter3D::MarkerShape3D::Sphere);
    CHECK_THAT(static_cast<double>(fix.scatter->markerSize()), WithinAbs(0.05, 1e-5));
    CHECK(fix.scatter->color() == QColor(Qt::blue));

    fix.scatter->setMarkerShape(Scatter3D::MarkerShape3D::Cube);
    CHECK(fix.scatter->markerShape() == Scatter3D::MarkerShape3D::Cube);

    fix.scatter->setMarkerSize(0.1f);
    CHECK_THAT(static_cast<double>(fix.scatter->markerSize()), WithinAbs(0.1, 1e-5));

    fix.scatter->setColor(Qt::green);
    CHECK(fix.scatter->color() == QColor(Qt::green));
}

TEST_CASE("Scatter3D: invalidate rebuilds cache", "[scatter3d]") {
    Scatter3DFixture fix;

    // Access positions to build the cache.
    const auto& pts = fix.scatter->pointPositions();
    CHECK(pts.size() == 3);

    // Invalidate and verify the cache rebuilds.
    fix.scatter->invalidate();
    const auto& pts2 = fix.scatter->pointPositions();
    CHECK(pts2.size() == 3);
    CHECK_THAT(static_cast<double>(pts2[0].x()), WithinAbs(1.0, 1e-5));
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Scatter3D tests skipped (no OpenGL widgets)", "[scatter3d]") {
    SUCCEED("OpenGL widgets not available -- skipping Scatter3D tests");
}

#endif
