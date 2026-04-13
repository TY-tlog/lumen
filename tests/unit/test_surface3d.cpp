// Unit tests for Surface3D.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Unit.h>
#include <plot3d/Ray.h>
#include <plot3d/Surface3D.h>

#include <cmath>
#include <memory>
#include <vector>

using namespace lumen::plot3d;
using namespace lumen::data;
using Catch::Matchers::WithinAbs;

namespace {

/// Create a flat 4x3 grid (4 columns, 3 rows) with z = x + y.
std::shared_ptr<Grid2D> makeFlatGrid()
{
    Dimension dimX{QStringLiteral("x"), Unit::dimensionless(), 4,
                   CoordinateArray(0.0, 1.0, 4)};
    Dimension dimY{QStringLiteral("y"), Unit::dimensionless(), 3,
                   CoordinateArray(0.0, 1.0, 3)};

    // 3 rows x 4 cols = 12 values, row-major.
    // z = x + y for each grid point.
    std::vector<double> data;
    for (std::size_t iy = 0; iy < 3; ++iy) {
        for (std::size_t ix = 0; ix < 4; ++ix) {
            data.push_back(static_cast<double>(ix + iy));
        }
    }

    return std::make_shared<Grid2D>(QStringLiteral("test_surface"),
                                    Unit::dimensionless(),
                                    std::move(dimX), std::move(dimY),
                                    std::move(data));
}

/// Create a sphere-like grid for normal testing.
/// Grid on [0, pi] x [0, 2*pi], z = sin(x)*cos(y).
std::shared_ptr<Grid2D> makeSphereGrid()
{
    const std::size_t nx = 10;
    const std::size_t ny = 10;
    Dimension dimX{QStringLiteral("x"), Unit::dimensionless(), nx,
                   CoordinateArray(0.0, M_PI / static_cast<double>(nx - 1), nx)};
    Dimension dimY{QStringLiteral("y"), Unit::dimensionless(), ny,
                   CoordinateArray(0.0, 2.0 * M_PI / static_cast<double>(ny - 1), ny)};

    std::vector<double> data;
    for (std::size_t iy = 0; iy < ny; ++iy) {
        for (std::size_t ix = 0; ix < nx; ++ix) {
            double x = static_cast<double>(ix) * M_PI / static_cast<double>(nx - 1);
            double y = static_cast<double>(iy) * 2.0 * M_PI / static_cast<double>(ny - 1);
            data.push_back(std::sin(x) * std::cos(y));
        }
    }

    return std::make_shared<Grid2D>(QStringLiteral("sphere_surface"),
                                    Unit::dimensionless(),
                                    std::move(dimX), std::move(dimY),
                                    std::move(data));
}

}  // namespace

TEST_CASE("Surface3D: mesh vertex count matches grid", "[surface3d]") {
    auto grid = makeFlatGrid();
    Surface3D surface(grid);

    // 4x3 grid = 12 vertices.
    CHECK(surface.vertices().size() == 12);

    // 2*(4-1)*(3-1) = 12 triangles.
    CHECK(surface.triangleCount() == 12);

    // Index count = 12 * 3 = 36.
    CHECK(surface.indices().size() == 36);
}

TEST_CASE("Surface3D: normals are valid on sphere grid", "[surface3d]") {
    auto grid = makeSphereGrid();
    Surface3D surface(grid);

    const auto& verts = surface.vertices();
    REQUIRE_FALSE(verts.empty());

    // Every vertex normal should be unit length (or close to it).
    for (const auto& v : verts) {
        float nlen = v.normal.length();
        CHECK_THAT(static_cast<double>(nlen), WithinAbs(1.0, 0.01));
    }
}

TEST_CASE("Surface3D: HeightMap mode sets z from grid values", "[surface3d]") {
    auto grid = makeFlatGrid();
    Surface3D surface(grid);
    surface.setMode(Surface3D::Mode::HeightMap);

    const auto& verts = surface.vertices();
    // Vertex at grid position (0,0) should have z=0.
    CHECK_THAT(static_cast<double>(verts[0].position.z()), WithinAbs(0.0, 1e-5));
    // Vertex at grid position (3,2) = data[2*4+3] = 5.
    CHECK_THAT(static_cast<double>(verts[2 * 4 + 3].position.z()), WithinAbs(5.0, 1e-5));
}

TEST_CASE("Surface3D: FlatColored mode sets z to zero", "[surface3d]") {
    auto grid = makeFlatGrid();
    Surface3D surface(grid);
    surface.setMode(Surface3D::Mode::FlatColored);

    const auto& verts = surface.vertices();
    for (const auto& v : verts) {
        CHECK_THAT(static_cast<double>(v.position.z()), WithinAbs(0.0, 1e-5));
    }
}

TEST_CASE("Surface3D: wireframe toggle", "[surface3d]") {
    auto grid = makeFlatGrid();
    Surface3D surface(grid);

    CHECK_FALSE(surface.wireframe());
    surface.setWireframe(true);
    CHECK(surface.wireframe());
}

TEST_CASE("Surface3D: dataBounds covers grid extent", "[surface3d]") {
    auto grid = makeFlatGrid();
    Surface3D surface(grid);

    auto bounds = surface.dataBounds();
    CHECK(bounds.isValid());
    CHECK_THAT(static_cast<double>(bounds.min.x()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.max.x()), WithinAbs(3.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.min.y()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.max.y()), WithinAbs(2.0, 1e-5));
}

TEST_CASE("Surface3D: hitTestRay detects surface", "[surface3d]") {
    auto grid = makeFlatGrid();
    Surface3D surface(grid);
    surface.setMode(Surface3D::Mode::HeightMap);

    // Shoot a ray straight down at the center of the grid.
    // Grid center is around x=1.5, y=1.0, z should be 2.5 (heightmap).
    Ray ray{QVector3D(1.5f, 1.0f, 20.0f), QVector3D(0, 0, -1)};
    auto hit = surface.hitTestRay(ray, 100.0);
    REQUIRE(hit.has_value());
    // Should hit near z = 2.5 (value at center).
    CHECK(hit->distance > 0.0);
}

TEST_CASE("Surface3D: invalidate triggers rebuild", "[surface3d]") {
    auto grid = makeFlatGrid();
    Surface3D surface(grid);

    // Build initial mesh.
    auto count1 = surface.triangleCount();
    CHECK(count1 == 12);

    // Invalidate and rebuild.
    surface.invalidate();
    auto count2 = surface.triangleCount();
    CHECK(count2 == 12);
}

TEST_CASE("Surface3D: type is Surface3D", "[surface3d]") {
    auto grid = makeFlatGrid();
    Surface3D surface(grid);
    CHECK(surface.type() == PlotItem3D::Type::Surface3D);
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Surface3D tests skipped (no OpenGL widgets)", "[surface3d]") {
    SUCCEED("OpenGL widgets not available -- skipping Surface3D tests");
}

#endif
