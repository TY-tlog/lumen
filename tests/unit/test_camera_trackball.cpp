// Unit tests for Camera trackball mode.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <plot3d/Camera.h>

#include <QVector3D>
#include <cmath>

using namespace lumen::plot3d;
using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinAbs;

TEST_CASE("Camera: default state is trackball mode", "[camera][trackball]") {
    Camera cam;
    CHECK(cam.mode() == CameraMode::Trackball);
}

TEST_CASE("Camera: default position is on +Z axis from origin", "[camera][trackball]") {
    Camera cam;
    QVector3D pos = cam.position();
    // Default: target at origin, distance 5, identity quaternion looks along +Z.
    CHECK(pos.z() > 0.0f);
    CHECK_THAT(static_cast<double>(pos.length()),
               WithinRel(5.0, 0.01));
}

TEST_CASE("Camera: trackball drag rotates around target", "[camera][trackball]") {
    Camera cam;
    QVector3D posBefore = cam.position();

    // Drag horizontally.
    cam.handleDrag(QPointF(100.0, 0.0));

    QVector3D posAfter = cam.position();

    // Position should change after drag.
    float dx = posBefore.x() - posAfter.x();
    float dy = posBefore.y() - posAfter.y();
    float dz = posBefore.z() - posAfter.z();
    double dist = std::sqrt(static_cast<double>(dx * dx + dy * dy + dz * dz));
    CHECK(dist > 0.01);

    // Distance to target should remain the same.
    QVector3D toTarget = posAfter - cam.target();
    CHECK_THAT(static_cast<double>(toTarget.length()),
               WithinRel(5.0, 0.01));
}

TEST_CASE("Camera: trackball drag preserves distance to target", "[camera][trackball]") {
    Camera cam;
    cam.setTarget(QVector3D(1, 2, 3));

    for (int i = 0; i < 10; ++i) {
        cam.handleDrag(QPointF(50.0, 30.0));
    }

    QVector3D diff = cam.position() - cam.target();
    CHECK_THAT(static_cast<double>(diff.length()),
               WithinRel(5.0, 0.01));
}

TEST_CASE("Camera: trackball drag with vertical delta rotates vertically", "[camera][trackball]") {
    Camera cam;
    QVector3D posBefore = cam.position();

    cam.handleDrag(QPointF(0.0, 100.0));

    QVector3D posAfter = cam.position();
    // Y component should change for vertical drag.
    CHECK(std::abs(posAfter.y() - posBefore.y()) > 0.01f);
}

TEST_CASE("Camera: wheel zoom changes distance", "[camera][trackball]") {
    Camera cam;
    float distBefore = cam.distance();

    cam.handleWheel(120.0);  // zoom in
    CHECK(cam.distance() < distBefore);

    cam.handleWheel(-240.0);  // zoom out
    CHECK(cam.distance() > distBefore);
}

TEST_CASE("Camera: view matrix is valid", "[camera][trackball]") {
    Camera cam;
    QMatrix4x4 view = cam.viewMatrix();
    // The view matrix should not be identity (camera is not at origin looking down -Z with up=Y).
    CHECK(!view.isIdentity());
}

TEST_CASE("Camera: projection matrix is valid perspective", "[camera][trackball]") {
    Camera cam;
    QMatrix4x4 proj = cam.projectionMatrix(1.5f);
    CHECK(!proj.isIdentity());
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Camera trackball tests skipped (no OpenGL widgets)", "[camera][trackball]") {
    SUCCEED("OpenGL widgets not available — skipping camera trackball tests");
}

#endif
