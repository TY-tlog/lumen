// Unit tests for Streamlines and RK4 integration.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Unit.h>
#include <data/Volume3D.h>
#include <plot3d/Streamlines.h>

#include <cmath>
#include <memory>

using namespace lumen::plot3d;
using namespace lumen::data;
using Catch::Matchers::WithinAbs;

namespace {

/// Create a circular vector field: v = (-y, x, 0) (normalized, scaled).
/// This should produce circular streamlines in the XY plane.
struct CircularFieldFixture {
    static constexpr std::size_t N = 20;

    Dimension makeDim(const QString& name)
    {
        return Dimension{name, Unit::dimensionless(), N,
                        CoordinateArray(-5.0, 10.0 / static_cast<double>(N - 1), N)};
    }

    std::shared_ptr<Volume3D> makeVx()
    {
        auto dimX = makeDim(QStringLiteral("x"));
        auto dimY = makeDim(QStringLiteral("y"));
        auto dimZ = makeDim(QStringLiteral("z"));

        std::vector<double> data(N * N * N);
        for (std::size_t iz = 0; iz < N; ++iz) {
            for (std::size_t iy = 0; iy < N; ++iy) {
                for (std::size_t ix = 0; ix < N; ++ix) {
                    double y = -5.0 + static_cast<double>(iy) * 10.0 / static_cast<double>(N - 1);
                    data[iz * N * N + iy * N + ix] = -y;  // vx = -y
                }
            }
        }

        return std::make_shared<Volume3D>(QStringLiteral("vx"),
                                          Unit::dimensionless(),
                                          std::move(dimX), std::move(dimY),
                                          std::move(dimZ), std::move(data));
    }

    std::shared_ptr<Volume3D> makeVy()
    {
        auto dimX = makeDim(QStringLiteral("x"));
        auto dimY = makeDim(QStringLiteral("y"));
        auto dimZ = makeDim(QStringLiteral("z"));

        std::vector<double> data(N * N * N);
        for (std::size_t iz = 0; iz < N; ++iz) {
            for (std::size_t iy = 0; iy < N; ++iy) {
                for (std::size_t ix = 0; ix < N; ++ix) {
                    double x = -5.0 + static_cast<double>(ix) * 10.0 / static_cast<double>(N - 1);
                    data[iz * N * N + iy * N + ix] = x;  // vy = x
                }
            }
        }

        return std::make_shared<Volume3D>(QStringLiteral("vy"),
                                          Unit::dimensionless(),
                                          std::move(dimX), std::move(dimY),
                                          std::move(dimZ), std::move(data));
    }

    std::shared_ptr<Volume3D> makeVz()
    {
        auto dimX = makeDim(QStringLiteral("x"));
        auto dimY = makeDim(QStringLiteral("y"));
        auto dimZ = makeDim(QStringLiteral("z"));

        std::vector<double> data(N * N * N, 0.0);  // vz = 0

        return std::make_shared<Volume3D>(QStringLiteral("vz"),
                                          Unit::dimensionless(),
                                          std::move(dimX), std::move(dimY),
                                          std::move(dimZ), std::move(data));
    }
};

}  // namespace

TEST_CASE("Streamlines: circular field produces near-circular streamlines", "[streamlines]") {
    CircularFieldFixture fix;
    auto vx = fix.makeVx();
    auto vy = fix.makeVy();
    auto vz = fix.makeVz();

    Streamlines sl(vx, vy, vz, "circular");
    sl.setIntegrationStep(0.01f);
    sl.setMaxSteps(2000);

    // Seed at (2, 0, 0) -> should trace a circle of radius 2.
    sl.setSeedPoints({QVector3D(2.0f, 0.0f, 0.0f)});

    const auto& lines = sl.lines();
    REQUIRE(lines.size() == 1);
    REQUIRE(lines[0].points.size() > 10);

    // Check that all points stay approximately at radius 2 from origin (in XY).
    for (const auto& p : lines[0].points) {
        double r = std::sqrt(static_cast<double>(p.x() * p.x() + p.y() * p.y()));
        CHECK_THAT(r, WithinAbs(2.0, 0.3));  // Allow for numerical integration error
    }
}

TEST_CASE("Streamlines: seed grid creates expected number of seeds", "[streamlines]") {
    CircularFieldFixture fix;
    auto vx = fix.makeVx();
    auto vy = fix.makeVy();
    auto vz = fix.makeVz();

    Streamlines sl(vx, vy, vz);
    sl.setSeedGrid(3);  // 3^3 = 27 seed points

    CHECK(sl.seedPoints().size() == 27);
}

TEST_CASE("Streamlines: type and visibility", "[streamlines]") {
    CircularFieldFixture fix;
    auto vx = fix.makeVx();
    auto vy = fix.makeVy();
    auto vz = fix.makeVz();

    Streamlines sl(vx, vy, vz, "test");
    CHECK(sl.type() == PlotItem3D::Type::Streamlines);
    CHECK(sl.isVisible());
    CHECK(sl.name() == QStringLiteral("test"));

    sl.setVisible(false);
    CHECK_FALSE(sl.isVisible());
}

TEST_CASE("Streamlines: integration parameters", "[streamlines]") {
    CircularFieldFixture fix;
    auto vx = fix.makeVx();
    auto vy = fix.makeVy();
    auto vz = fix.makeVz();

    Streamlines sl(vx, vy, vz);
    CHECK_THAT(static_cast<double>(sl.integrationStep()), WithinAbs(0.05, 1e-5));
    CHECK(sl.maxSteps() == 500);

    sl.setIntegrationStep(0.1f);
    sl.setMaxSteps(100);

    CHECK_THAT(static_cast<double>(sl.integrationStep()), WithinAbs(0.1, 1e-5));
    CHECK(sl.maxSteps() == 100);
}

TEST_CASE("Streamlines: trilinear interpolation at grid points", "[streamlines]") {
    CircularFieldFixture fix;
    auto vx = fix.makeVx();
    auto vy = fix.makeVy();
    auto vz = fix.makeVz();

    Streamlines sl(vx, vy, vz);

    // At (0, 0, 0): vx should be ~0, vy should be ~0.
    QVector3D v = sl.sampleField(QVector3D(0, 0, 0));
    CHECK_THAT(static_cast<double>(v.x()), WithinAbs(0.0, 0.3));
    CHECK_THAT(static_cast<double>(v.y()), WithinAbs(0.0, 0.3));
    CHECK_THAT(static_cast<double>(v.z()), WithinAbs(0.0, 0.01));

    // At (2, 0, 0): vx should be ~0, vy should be ~2.
    QVector3D v2 = sl.sampleField(QVector3D(2.0f, 0, 0));
    CHECK_THAT(static_cast<double>(v2.x()), WithinAbs(0.0, 0.3));
    CHECK_THAT(static_cast<double>(v2.y()), WithinAbs(2.0, 0.3));
}

TEST_CASE("Streamlines: empty seeds produce no lines", "[streamlines]") {
    CircularFieldFixture fix;
    auto vx = fix.makeVx();
    auto vy = fix.makeVy();
    auto vz = fix.makeVz();

    Streamlines sl(vx, vy, vz);
    // No seeds set.
    CHECK(sl.lines().empty());
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Streamlines tests skipped (no OpenGL widgets)", "[streamlines]") {
    SUCCEED("OpenGL widgets not available -- skipping Streamlines tests");
}

#endif
