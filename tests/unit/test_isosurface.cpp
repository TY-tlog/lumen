// Unit tests for Isosurface and Marching Cubes.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Unit.h>
#include <data/Volume3D.h>
#include <plot3d/Isosurface.h>
#include <plot3d/MarchingCubes.h>

#include <cmath>
#include <memory>

using namespace lumen::plot3d;
using namespace lumen::data;
using Catch::Matchers::WithinAbs;

namespace {

/// Create a volume containing a sphere: f(x,y,z) = x^2 + y^2 + z^2.
/// Domain: [-2, 2]^3, resolution N^3.
std::shared_ptr<Volume3D> makeSphereVolume(std::size_t N = 20)
{
    double lo = -2.0;
    double hi = 2.0;
    double step = (hi - lo) / static_cast<double>(N - 1);

    Dimension dimX{QStringLiteral("x"), Unit::dimensionless(), N,
                   CoordinateArray(lo, step, N)};
    Dimension dimY{QStringLiteral("y"), Unit::dimensionless(), N,
                   CoordinateArray(lo, step, N)};
    Dimension dimZ{QStringLiteral("z"), Unit::dimensionless(), N,
                   CoordinateArray(lo, step, N)};

    std::vector<double> data(N * N * N);
    for (std::size_t iz = 0; iz < N; ++iz) {
        double z = lo + static_cast<double>(iz) * step;
        for (std::size_t iy = 0; iy < N; ++iy) {
            double y = lo + static_cast<double>(iy) * step;
            for (std::size_t ix = 0; ix < N; ++ix) {
                double x = lo + static_cast<double>(ix) * step;
                data[iz * N * N + iy * N + ix] = x * x + y * y + z * z;
            }
        }
    }

    return std::make_shared<Volume3D>(QStringLiteral("sphere_field"),
                                     Unit::dimensionless(),
                                     std::move(dimX), std::move(dimY),
                                     std::move(dimZ), std::move(data));
}

}  // namespace

TEST_CASE("MarchingCubes: sphere mesh has expected vertex count", "[marching_cubes]") {
    auto vol = makeSphereVolume(20);

    // Extract isosurface at r^2 = 1.0 (unit sphere).
    auto mesh = MarchingCubes::extract(*vol, 1.0);

    // A 20^3 grid with a unit sphere should produce a reasonable number of triangles.
    // The exact count depends on the grid resolution, but it should be > 100 and < 5000.
    CHECK(mesh.triangleCount() > 100);
    CHECK(mesh.triangleCount() < 5000);
    CHECK_FALSE(mesh.empty());
}

TEST_CASE("MarchingCubes: sphere surface area within tolerance", "[marching_cubes]") {
    auto vol = makeSphereVolume(30);

    // Extract isosurface at r^2 = 1.0 (unit sphere).
    auto mesh = MarchingCubes::extract(*vol, 1.0);

    // Compute total surface area of the mesh.
    double totalArea = 0.0;
    for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        QVector3D v0 = mesh.vertices[mesh.indices[i]].position;
        QVector3D v1 = mesh.vertices[mesh.indices[i + 1]].position;
        QVector3D v2 = mesh.vertices[mesh.indices[i + 2]].position;

        QVector3D cross = QVector3D::crossProduct(v1 - v0, v2 - v0);
        totalArea += 0.5 * static_cast<double>(cross.length());
    }

    // Surface area of unit sphere = 4*pi ~= 12.566
    // With a 30^3 grid, we should be within ~15% tolerance.
    CHECK_THAT(totalArea, WithinAbs(4.0 * M_PI, 4.0 * M_PI * 0.15));
}

TEST_CASE("MarchingCubes: vertices lie on isosurface", "[marching_cubes]") {
    auto vol = makeSphereVolume(20);
    auto mesh = MarchingCubes::extract(*vol, 1.0);

    // Sample a few vertices and check they're approximately on the unit sphere.
    std::size_t checkCount = std::min(mesh.vertices.size(), static_cast<std::size_t>(50));
    std::size_t step = mesh.vertices.size() / checkCount;

    for (std::size_t i = 0; i < mesh.vertices.size(); i += step) {
        const auto& v = mesh.vertices[i];
        double r2 = static_cast<double>(v.position.x() * v.position.x() +
                                        v.position.y() * v.position.y() +
                                        v.position.z() * v.position.z());
        // Should be close to the iso value of 1.0.
        CHECK_THAT(r2, WithinAbs(1.0, 0.15));
    }
}

TEST_CASE("Isosurface: multiple iso values produce multiple meshes", "[isosurface]") {
    auto vol = makeSphereVolume(15);
    Isosurface iso(vol, "multi_iso");

    // Set two iso values: r^2=1.0 and r^2=2.0.
    iso.setIsoValues({1.0, 2.0});
    iso.setColors({QColor(Qt::red), QColor(Qt::blue)});

    const auto& meshes = iso.meshes();
    REQUIRE(meshes.size() == 2);
    CHECK(meshes[0].triangleCount() > 0);
    CHECK(meshes[1].triangleCount() > 0);

    // The outer surface (r^2=2.0) should have more or comparable triangles.
    CHECK(iso.totalTriangleCount() > meshes[0].triangleCount());
}

TEST_CASE("Isosurface: type and accessors", "[isosurface]") {
    auto vol = makeSphereVolume(10);
    Isosurface iso(vol, "test_iso");

    CHECK(iso.type() == PlotItem3D::Type::Isosurface);
    CHECK(iso.name() == QStringLiteral("test_iso"));
    CHECK(iso.isVisible());

    iso.setVisible(false);
    CHECK_FALSE(iso.isVisible());
}

TEST_CASE("Isosurface: dataBounds from volume", "[isosurface]") {
    auto vol = makeSphereVolume(10);
    Isosurface iso(vol);

    auto bounds = iso.dataBounds();
    CHECK(bounds.isValid());
    CHECK_THAT(static_cast<double>(bounds.min.x()), WithinAbs(-2.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.max.x()), WithinAbs(2.0, 1e-5));
}

TEST_CASE("Isosurface: empty iso values produce no meshes", "[isosurface]") {
    auto vol = makeSphereVolume(10);
    Isosurface iso(vol);

    // No iso values set.
    CHECK(iso.meshes().empty());
    CHECK(iso.totalTriangleCount() == 0);
}

TEST_CASE("MarchingCubes: no surface for iso value outside data range", "[marching_cubes]") {
    auto vol = makeSphereVolume(10);

    // All values are r^2, ranging from 0 to ~12.
    // Iso value of 100 should produce empty mesh.
    auto mesh = MarchingCubes::extract(*vol, 100.0);
    CHECK(mesh.empty());
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Isosurface tests skipped (no OpenGL widgets)", "[isosurface]") {
    SUCCEED("OpenGL widgets not available -- skipping Isosurface tests");
}

#endif
