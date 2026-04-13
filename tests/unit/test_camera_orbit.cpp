// Unit tests for Camera orbit mode.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <plot3d/Camera.h>

#include <QVector3D>
#include <cmath>

using namespace lumen::plot3d;
using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinAbs;

namespace {

constexpr double kMaxElevationDeg = 89.0;
constexpr double kMaxElevationRad = kMaxElevationDeg * M_PI / 180.0;

}  // namespace

TEST_CASE("Camera: orbit mode set correctly", "[camera][orbit]") {
    Camera cam;
    cam.setMode(CameraMode::Orbit);
    CHECK(cam.mode() == CameraMode::Orbit);
}

TEST_CASE("Camera: orbit drag changes azimuth", "[camera][orbit]") {
    Camera cam;
    cam.setMode(CameraMode::Orbit);
    QVector3D posBefore = cam.position();

    cam.handleDrag(QPointF(200.0, 0.0));

    QVector3D posAfter = cam.position();
    // X or Z position should change with horizontal drag.
    float dx = std::abs(posAfter.x() - posBefore.x());
    float dz = std::abs(posAfter.z() - posBefore.z());
    CHECK((dx + dz) > 0.01f);
}

TEST_CASE("Camera: orbit drag changes elevation", "[camera][orbit]") {
    Camera cam;
    cam.setMode(CameraMode::Orbit);
    QVector3D posBefore = cam.position();

    cam.handleDrag(QPointF(0.0, 200.0));

    QVector3D posAfter = cam.position();
    // Y position should change with vertical drag.
    CHECK(std::abs(posAfter.y() - posBefore.y()) > 0.01f);
}

TEST_CASE("Camera: orbit elevation is clamped to +-89 degrees", "[camera][orbit]") {
    Camera cam;
    cam.setMode(CameraMode::Orbit);

    // Drag far upward — should clamp elevation.
    for (int i = 0; i < 100; ++i) {
        cam.handleDrag(QPointF(0.0, 500.0));
    }

    QVector3D pos = cam.position();
    QVector3D diff = pos - cam.target();
    float elevation = std::asin(std::clamp(diff.normalized().y(), -1.0f, 1.0f));

    CHECK_THAT(static_cast<double>(elevation),
               WithinAbs(kMaxElevationRad, 0.02));
}

TEST_CASE("Camera: orbit negative elevation clamped", "[camera][orbit]") {
    Camera cam;
    cam.setMode(CameraMode::Orbit);

    // Drag far downward.
    for (int i = 0; i < 100; ++i) {
        cam.handleDrag(QPointF(0.0, -500.0));
    }

    QVector3D pos = cam.position();
    QVector3D diff = pos - cam.target();
    float elevation = std::asin(std::clamp(diff.normalized().y(), -1.0f, 1.0f));

    CHECK_THAT(static_cast<double>(elevation),
               WithinAbs(-kMaxElevationRad, 0.02));
}

TEST_CASE("Camera: orbit preserves distance to target", "[camera][orbit]") {
    Camera cam;
    cam.setMode(CameraMode::Orbit);

    for (int i = 0; i < 20; ++i) {
        cam.handleDrag(QPointF(30.0, 20.0));
    }

    QVector3D diff = cam.position() - cam.target();
    CHECK_THAT(static_cast<double>(diff.length()),
               WithinRel(5.0, 0.01));
}

TEST_CASE("Camera: orbit up is always world-up", "[camera][orbit]") {
    Camera cam;
    cam.setMode(CameraMode::Orbit);

    cam.handleDrag(QPointF(100.0, 50.0));

    QVector3D upDir = cam.up();
    CHECK_THAT(static_cast<double>(upDir.x()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(upDir.y()), WithinAbs(1.0, 1e-5));
    CHECK_THAT(static_cast<double>(upDir.z()), WithinAbs(0.0, 1e-5));
}

TEST_CASE("Camera: pan translates target in orbit mode", "[camera][orbit]") {
    Camera cam;
    cam.setMode(CameraMode::Orbit);
    QVector3D targetBefore = cam.target();

    cam.handlePan(QPointF(100.0, 0.0));

    QVector3D targetAfter = cam.target();
    QVector3D diff = targetAfter - targetBefore;
    CHECK(diff.length() > 0.01f);
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Camera orbit tests skipped (no OpenGL widgets)", "[camera][orbit]") {
    SUCCEED("OpenGL widgets not available — skipping camera orbit tests");
}

#endif
