#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "plot3d/Camera.h"

#include <QVector3D>

#include <cmath>

using lumen::plot3d::Camera;
using lumen::plot3d::CameraMode;
using Catch::Matchers::WithinAbs;

TEST_CASE("Camera: trackball to orbit preserves approximate position",
          "[camera][mode_switch]") {
    Camera cam;
    cam.setMode(CameraMode::Trackball);
    cam.setTarget(QVector3D(0, 0, 0));
    cam.setPosition(QVector3D(3, 4, 5));
    QVector3D posBefore = cam.position();

    cam.setMode(CameraMode::Orbit);
    QVector3D posAfter = cam.position();

    // Position should be approximately preserved after mode switch.
    CHECK_THAT(static_cast<double>((posAfter - posBefore).length()),
               WithinAbs(0.0, 0.5));
}

TEST_CASE("Camera: orbit to trackball preserves approximate position",
          "[camera][mode_switch]") {
    Camera cam;
    cam.setMode(CameraMode::Orbit);
    cam.setTarget(QVector3D(0, 0, 0));
    cam.setPosition(QVector3D(2, 3, 4));
    QVector3D posBefore = cam.position();

    cam.setMode(CameraMode::Trackball);
    QVector3D posAfter = cam.position();

    CHECK_THAT(static_cast<double>((posAfter - posBefore).length()),
               WithinAbs(0.0, 0.5));
}

TEST_CASE("Camera: same mode switch is no-op",
          "[camera][mode_switch]") {
    Camera cam;
    cam.setMode(CameraMode::Trackball);
    cam.setPosition(QVector3D(1, 2, 3));
    QVector3D pos1 = cam.position();

    cam.setMode(CameraMode::Trackball);  // same mode
    QVector3D pos2 = cam.position();

    CHECK_THAT(static_cast<double>((pos2 - pos1).length()),
               WithinAbs(0.0, 1e-5));
}

TEST_CASE("Camera: round-trip trackball->orbit->trackball",
          "[camera][mode_switch]") {
    Camera cam;
    cam.setMode(CameraMode::Trackball);
    cam.setPosition(QVector3D(5, 0, 0));
    QVector3D posOrig = cam.position();

    cam.setMode(CameraMode::Orbit);
    cam.setMode(CameraMode::Trackball);
    QVector3D posAfter = cam.position();

    // Approximate preservation after round-trip.
    CHECK_THAT(static_cast<double>((posAfter - posOrig).length()),
               WithinAbs(0.0, 1.0));
}

TEST_CASE("Camera: orbit up vector is world-up",
          "[camera][mode_switch]") {
    Camera cam;
    cam.setMode(CameraMode::Orbit);
    QVector3D up = cam.up();
    CHECK_THAT(static_cast<double>(up.x()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(up.y()), WithinAbs(1.0, 1e-5));
    CHECK_THAT(static_cast<double>(up.z()), WithinAbs(0.0, 1e-5));
}

TEST_CASE("Camera: distance preserved across mode switch",
          "[camera][mode_switch]") {
    Camera cam;
    cam.setMode(CameraMode::Trackball);
    cam.setTarget(QVector3D(0, 0, 0));
    cam.setPosition(QVector3D(0, 0, 7));

    float distBefore = (cam.position() - QVector3D(0, 0, 0)).length();
    cam.setMode(CameraMode::Orbit);
    float distAfter = (cam.position() - QVector3D(0, 0, 0)).length();

    CHECK_THAT(static_cast<double>(distAfter),
               WithinAbs(static_cast<double>(distBefore), 0.1));
}
