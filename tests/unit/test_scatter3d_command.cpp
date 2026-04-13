// Unit tests for ChangeScatter3DPropertiesCommand.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <core/CommandBus.h>
#include <core/commands/ChangeScatter3DPropertiesCommand.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <plot3d/Scatter3D.h>
#include <plot3d/Scene3D.h>

#include <memory>
#include <vector>

using namespace lumen::plot3d;
using lumen::core::CommandBus;
using lumen::core::commands::ChangeScatter3DPropertiesCommand;
using lumen::data::Rank1Dataset;
using lumen::data::Unit;
using Catch::Matchers::WithinAbs;

namespace {

struct Scatter3DCommandFixture {
    std::shared_ptr<Rank1Dataset> xDs = std::make_shared<Rank1Dataset>(
        "x", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0});
    std::shared_ptr<Rank1Dataset> yDs = std::make_shared<Rank1Dataset>(
        "y", Unit::dimensionless(), std::vector<double>{4.0, 5.0, 6.0});
    std::shared_ptr<Rank1Dataset> zDs = std::make_shared<Rank1Dataset>(
        "z", Unit::dimensionless(), std::vector<double>{7.0, 8.0, 9.0});
    std::unique_ptr<Scene3D> scene = std::make_unique<Scene3D>();

    Scatter3DCommandFixture() {
        scene->addItem(std::make_unique<Scatter3D>(
            xDs, yDs, zDs, Qt::blue, "scatter3d_1"));
    }
};

}  // namespace

TEST_CASE("ChangeScatter3DPropertiesCommand: execute applies new properties",
          "[core][scatter3d_command]") {
    Scatter3DCommandFixture fix;

    auto cmd = std::make_unique<ChangeScatter3DPropertiesCommand>(
        fix.scene.get(), 0,
        Scatter3D::MarkerShape3D::Cube, 0.2f,
        QColor(Qt::red), "renamed", false);

    cmd->execute();

    auto* scatter = dynamic_cast<Scatter3D*>(fix.scene->itemAt(0));
    REQUIRE(scatter != nullptr);
    CHECK(scatter->markerShape() == Scatter3D::MarkerShape3D::Cube);
    CHECK_THAT(static_cast<double>(scatter->markerSize()), WithinAbs(0.2, 1e-5));
    CHECK(scatter->color() == QColor(Qt::red));
    CHECK(scatter->name() == QStringLiteral("renamed"));
    CHECK_FALSE(scatter->isVisible());
}

TEST_CASE("ChangeScatter3DPropertiesCommand: undo restores old properties",
          "[core][scatter3d_command]") {
    Scatter3DCommandFixture fix;

    auto cmd = std::make_unique<ChangeScatter3DPropertiesCommand>(
        fix.scene.get(), 0,
        Scatter3D::MarkerShape3D::Tetrahedron, 0.3f,
        QColor(Qt::green), "changed", false);

    cmd->execute();
    auto* scatter = dynamic_cast<Scatter3D*>(fix.scene->itemAt(0));
    REQUIRE(scatter != nullptr);
    CHECK(scatter->markerShape() == Scatter3D::MarkerShape3D::Tetrahedron);

    cmd->undo();
    CHECK(scatter->markerShape() == Scatter3D::MarkerShape3D::Sphere);
    CHECK_THAT(static_cast<double>(scatter->markerSize()), WithinAbs(0.05, 1e-5));
    CHECK(scatter->color() == QColor(Qt::blue));
    CHECK(scatter->name() == QStringLiteral("scatter3d_1"));
    CHECK(scatter->isVisible());
}

TEST_CASE("ChangeScatter3DPropertiesCommand: round-trip through CommandBus",
          "[core][scatter3d_command]") {
    Scatter3DCommandFixture fix;

    CommandBus bus;

    bus.execute(std::make_unique<ChangeScatter3DPropertiesCommand>(
        fix.scene.get(), 0,
        Scatter3D::MarkerShape3D::Cube, 0.15f,
        QColor(Qt::cyan), "bus_test", true));

    auto* scatter = dynamic_cast<Scatter3D*>(fix.scene->itemAt(0));
    REQUIRE(scatter != nullptr);
    CHECK(scatter->color() == QColor(Qt::cyan));
    CHECK(scatter->markerShape() == Scatter3D::MarkerShape3D::Cube);

    bus.undo();
    CHECK(scatter->color() == QColor(Qt::blue));
    CHECK(scatter->markerShape() == Scatter3D::MarkerShape3D::Sphere);

    bus.redo();
    CHECK(scatter->color() == QColor(Qt::cyan));
}

TEST_CASE("ChangeScatter3DPropertiesCommand: description is non-empty",
          "[core][scatter3d_command]") {
    Scatter3DCommandFixture fix;

    auto cmd = std::make_unique<ChangeScatter3DPropertiesCommand>(
        fix.scene.get(), 0,
        Scatter3D::MarkerShape3D::Sphere, 0.05f,
        QColor(Qt::blue), "scatter3d_1", true);

    CHECK_FALSE(cmd->description().isEmpty());
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("ChangeScatter3DPropertiesCommand tests skipped (no OpenGL widgets)",
          "[scatter3d_command]") {
    SUCCEED("OpenGL widgets not available -- skipping ChangeScatter3DPropertiesCommand tests");
}

#endif
