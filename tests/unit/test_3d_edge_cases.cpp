#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <data/Volume3D.h>
#include <plot3d/BoundingBox3D.h>
#include <plot3d/Scatter3D.h>
#include <plot3d/Scene3D.h>
#include <plot3d/Surface3D.h>
#include <plot3d/VolumeItem.h>

#include <memory>
#include <vector>

using namespace lumen::plot3d;
using lumen::data::CoordinateArray;
using lumen::data::Dimension;
using lumen::data::Grid2D;
using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using lumen::data::Volume3D;
using Catch::Matchers::WithinAbs;

namespace {

std::shared_ptr<Grid2D> makeGrid(std::size_t n)
{
    Dimension dimX{QStringLiteral("x"), Unit::dimensionless(), n,
                   CoordinateArray(0.0, 1.0, n)};
    Dimension dimY{QStringLiteral("y"), Unit::dimensionless(), n,
                   CoordinateArray(0.0, 1.0, n)};
    std::vector<double> data(n * n, 0.0);
    for (std::size_t i = 0; i < n * n; ++i)
        data[i] = static_cast<double>(i) / static_cast<double>(n * n);
    return std::make_shared<Grid2D>(
        QStringLiteral("test"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));
}

std::shared_ptr<Volume3D> makeVolume(std::size_t n)
{
    Dimension dimX{QStringLiteral("x"), Unit::dimensionless(), n,
                   CoordinateArray(0.0, 1.0, n)};
    Dimension dimY{QStringLiteral("y"), Unit::dimensionless(), n,
                   CoordinateArray(0.0, 1.0, n)};
    Dimension dimZ{QStringLiteral("z"), Unit::dimensionless(), n,
                   CoordinateArray(0.0, 1.0, n)};
    std::vector<double> data(n * n * n, 0.5);
    return std::make_shared<Volume3D>(
        QStringLiteral("vol"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(dimZ), std::move(data));
}

}  // namespace

TEST_CASE("Scatter3D: single point has valid bounding box",
          "[plot3d][edge_cases]") {
    auto x = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{1.0});
    auto y = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{2.0});
    auto z = std::make_shared<Rank1Dataset>("z", Unit::dimensionless(), std::vector<double>{3.0});
    Scatter3D scatter(x, y, z, Qt::red, "single");
    auto bb = scatter.dataBounds();
    CHECK_THAT(static_cast<double>(bb.center().x()), WithinAbs(1.0, 1e-5));
    CHECK_THAT(static_cast<double>(bb.center().y()), WithinAbs(2.0, 1e-5));
    CHECK_THAT(static_cast<double>(bb.center().z()), WithinAbs(3.0, 1e-5));
}

TEST_CASE("Scatter3D: visibility toggle", "[plot3d][edge_cases]") {
    auto x = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{0.0});
    auto y = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{0.0});
    auto z = std::make_shared<Rank1Dataset>("z", Unit::dimensionless(), std::vector<double>{0.0});
    Scatter3D scatter(x, y, z);
    CHECK(scatter.isVisible());
    scatter.setVisible(false);
    CHECK_FALSE(scatter.isVisible());
    scatter.setVisible(true);
    CHECK(scatter.isVisible());
}

TEST_CASE("Surface3D: small grid bounding box is valid",
          "[plot3d][edge_cases]") {
    auto grid = makeGrid(4);
    Surface3D surface(grid, Qt::blue, "small");
    auto bb = surface.dataBounds();
    // Bounds should be non-degenerate.
    CHECK(bb.size().x() > 0.0f);
    CHECK(bb.size().y() >= 0.0f);
    CHECK(bb.size().z() > 0.0f);
}

TEST_CASE("Surface3D: name setter", "[plot3d][edge_cases]") {
    auto grid = makeGrid(4);
    Surface3D surface(grid, Qt::blue, "original");
    CHECK(surface.name() == QStringLiteral("original"));
    surface.setName("changed");
    CHECK(surface.name() == QStringLiteral("changed"));
}

TEST_CASE("VolumeItem: visibility and name", "[plot3d][edge_cases]") {
    auto vol = makeVolume(4);
    VolumeItem item(vol, "vol1");
    CHECK(item.name() == QStringLiteral("vol1"));
    CHECK(item.isVisible());
    item.setVisible(false);
    CHECK_FALSE(item.isVisible());
}

TEST_CASE("Scene3D: clearItems empties the scene", "[plot3d][edge_cases]") {
    Scene3D scene;
    auto x = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{0.0});
    auto y = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{0.0});
    auto z = std::make_shared<Rank1Dataset>("z", Unit::dimensionless(), std::vector<double>{0.0});
    scene.addItem(std::make_unique<Scatter3D>(x, y, z));
    CHECK(scene.itemCount() > 0);
    scene.clearItems();
    CHECK(scene.itemCount() == 0);
}

TEST_CASE("Scene3D: multiple items", "[plot3d][edge_cases]") {
    Scene3D scene;
    auto x = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{0.0});
    auto y = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{0.0});
    auto z = std::make_shared<Rank1Dataset>("z", Unit::dimensionless(), std::vector<double>{0.0});
    scene.addItem(std::make_unique<Scatter3D>(x, y, z, Qt::red, "a"));
    scene.addItem(std::make_unique<Scatter3D>(x, y, z, Qt::blue, "b"));
    CHECK(scene.itemCount() == 2);
    CHECK(scene.itemAt(0)->name() == QStringLiteral("a"));
    CHECK(scene.itemAt(1)->name() == QStringLiteral("b"));
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("3D edge case tests skipped (no OpenGL widgets)",
          "[plot3d][edge_cases]") {
    SUCCEED("OpenGL widgets not available -- skipping 3D edge case tests");
}

#endif
