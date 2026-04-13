// Unit tests for SpatialGrid3D.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <plot3d/Ray.h>
#include <plot3d/SpatialGrid3D.h>

#include <vector>

using namespace lumen::plot3d;
using Catch::Matchers::WithinAbs;

TEST_CASE("SpatialGrid3D: empty grid", "[spatial_grid3d]") {
    std::vector<QVector3D> points;
    SpatialGrid3D grid(points, 1.0f);

    CHECK(grid.pointCount() == 0);
    CHECK(grid.occupiedCellCount() == 0);
}

TEST_CASE("SpatialGrid3D: single point", "[spatial_grid3d]") {
    std::vector<QVector3D> points = {QVector3D(0, 0, 0)};
    SpatialGrid3D grid(points, 1.0f);

    CHECK(grid.pointCount() == 1);
    CHECK(grid.occupiedCellCount() == 1);
}

TEST_CASE("SpatialGrid3D: multiple points in same cell", "[spatial_grid3d]") {
    std::vector<QVector3D> points = {
        QVector3D(0.1f, 0.1f, 0.1f),
        QVector3D(0.2f, 0.2f, 0.2f),
        QVector3D(0.3f, 0.3f, 0.3f)};
    SpatialGrid3D grid(points, 1.0f);

    CHECK(grid.pointCount() == 3);
    CHECK(grid.occupiedCellCount() == 1);
}

TEST_CASE("SpatialGrid3D: points in different cells", "[spatial_grid3d]") {
    std::vector<QVector3D> points = {
        QVector3D(0, 0, 0),
        QVector3D(5, 5, 5),
        QVector3D(-5, -5, -5)};
    SpatialGrid3D grid(points, 1.0f);

    CHECK(grid.pointCount() == 3);
    CHECK(grid.occupiedCellCount() == 3);
}

TEST_CASE("SpatialGrid3D: queryRay hits point on axis", "[spatial_grid3d]") {
    std::vector<QVector3D> points = {QVector3D(0, 0, 0)};
    SpatialGrid3D grid(points, 1.0f);

    // Ray along -Z axis passing through origin.
    Ray ray{QVector3D(0, 0, 10), QVector3D(0, 0, -1)};

    auto result = grid.queryRay(ray, 0.5f, 100.0f);
    REQUIRE(result.has_value());
    CHECK(result->pointIndex == 0);
    CHECK_THAT(static_cast<double>(result->distance), WithinAbs(9.5, 0.1));
}

TEST_CASE("SpatialGrid3D: queryRay misses when ray is too far", "[spatial_grid3d]") {
    std::vector<QVector3D> points = {QVector3D(5, 5, 5)};
    SpatialGrid3D grid(points, 1.0f);

    // Ray along -Z axis - misses point at (5,5,5).
    Ray ray{QVector3D(0, 0, 10), QVector3D(0, 0, -1)};

    auto result = grid.queryRay(ray, 0.1f, 100.0f);
    CHECK_FALSE(result.has_value());
}

TEST_CASE("SpatialGrid3D: queryRay finds nearest of two", "[spatial_grid3d]") {
    std::vector<QVector3D> points = {
        QVector3D(0, 0, 2),
        QVector3D(0, 0, 8)};
    SpatialGrid3D grid(points, 1.0f);

    // Ray from z=20 toward -Z.
    Ray ray{QVector3D(0, 0, 20), QVector3D(0, 0, -1)};

    auto result = grid.queryRay(ray, 0.5f, 100.0f);
    REQUIRE(result.has_value());
    // Should hit point at z=8 first (closer to ray origin at z=20).
    CHECK(result->pointIndex == 1);
    CHECK_THAT(static_cast<double>(result->distance), WithinAbs(11.5, 0.1));
}

TEST_CASE("SpatialGrid3D: queryRay respects maxDist", "[spatial_grid3d]") {
    std::vector<QVector3D> points = {QVector3D(0, 0, 0)};
    SpatialGrid3D grid(points, 1.0f);

    // Ray along -Z axis, but maxDist is very small.
    Ray ray{QVector3D(0, 0, 10), QVector3D(0, 0, -1)};

    auto result = grid.queryRay(ray, 0.5f, 5.0f);
    // Point is 10 units away, maxDist is 5 - should miss.
    CHECK_FALSE(result.has_value());
}

TEST_CASE("SpatialGrid3D: handles large point clouds", "[spatial_grid3d]") {
    // Build a grid of 10x10x10 = 1000 points.
    std::vector<QVector3D> points;
    for (int x = 0; x < 10; ++x) {
        for (int y = 0; y < 10; ++y) {
            for (int z = 0; z < 10; ++z) {
                points.emplace_back(static_cast<float>(x),
                                    static_cast<float>(y),
                                    static_cast<float>(z));
            }
        }
    }

    SpatialGrid3D grid(points, 2.0f);
    CHECK(grid.pointCount() == 1000);

    // Ray aimed at point (5, 5, 5) from far away.
    Ray ray{QVector3D(5, 5, 50), QVector3D(0, 0, -1)};
    auto result = grid.queryRay(ray, 0.2f, 100.0f);
    REQUIRE(result.has_value());

    // The hit should be at or near z=9 (the furthest z-layer along the ray).
    const QVector3D& hitPt = points[result->pointIndex];
    CHECK_THAT(static_cast<double>(hitPt.x()), WithinAbs(5.0, 0.1));
    CHECK_THAT(static_cast<double>(hitPt.y()), WithinAbs(5.0, 0.1));
    CHECK(hitPt.z() == 9.0f);
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("SpatialGrid3D tests skipped (no OpenGL widgets)", "[spatial_grid3d]") {
    SUCCEED("OpenGL widgets not available -- skipping SpatialGrid3D tests");
}

#endif
