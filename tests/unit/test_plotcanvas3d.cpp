// Unit tests for PlotCanvas3D.
// Note: GL context creation typically requires a display server.
// Under QT_QPA_PLATFORM=offscreen the widget can be created but
// initializeGL may not succeed. Tests here validate construction
// and non-GL logic.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>

#include <QApplication>

#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <plot3d/Camera.h>
#include <plot3d/Scatter3D.h>
#include <plot3d/Scene3D.h>
#include <ui/PlotCanvas3D.h>

#include <memory>
#include <vector>

using namespace lumen::ui;
using namespace lumen::plot3d;
using lumen::data::Rank1Dataset;
using lumen::data::Unit;

TEST_CASE("PlotCanvas3D: construction creates scene and camera", "[plotcanvas3d]") {
    // Ensure QApplication exists (required for QWidget).
    int argc = 0;
    char* argv[] = {nullptr};
    QApplication app(argc, argv);

    PlotCanvas3D canvas;
    CHECK(canvas.scene() != nullptr);
    CHECK(canvas.camera() != nullptr);
    CHECK(canvas.scene()->itemCount() == 0);
}

TEST_CASE("PlotCanvas3D: addItem adds to scene", "[plotcanvas3d]") {
    int argc = 0;
    char* argv[] = {nullptr};
    QApplication app(argc, argv);

    PlotCanvas3D canvas;

    auto xDs = std::make_shared<Rank1Dataset>(
        "x", Unit::dimensionless(), std::vector<double>{1.0, 2.0});
    auto yDs = std::make_shared<Rank1Dataset>(
        "y", Unit::dimensionless(), std::vector<double>{3.0, 4.0});
    auto zDs = std::make_shared<Rank1Dataset>(
        "z", Unit::dimensionless(), std::vector<double>{5.0, 6.0});

    canvas.addItem(std::make_unique<Scatter3D>(xDs, yDs, zDs));
    CHECK(canvas.scene()->itemCount() == 1);
}

TEST_CASE("PlotCanvas3D: setCameraMode changes mode", "[plotcanvas3d]") {
    int argc = 0;
    char* argv[] = {nullptr};
    QApplication app(argc, argv);

    PlotCanvas3D canvas;
    CHECK(canvas.camera()->mode() == CameraMode::Trackball);

    canvas.setCameraMode(CameraMode::Orbit);
    CHECK(canvas.camera()->mode() == CameraMode::Orbit);
}

TEST_CASE("PlotCanvas3D: scene has default lights", "[plotcanvas3d]") {
    int argc = 0;
    char* argv[] = {nullptr};
    QApplication app(argc, argv);

    PlotCanvas3D canvas;
    CHECK(canvas.scene()->lights().size() == 2);
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("PlotCanvas3D tests skipped (no OpenGL widgets)", "[plotcanvas3d]") {
    SUCCEED("OpenGL widgets not available -- skipping PlotCanvas3D tests");
}

#endif
