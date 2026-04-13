// Unit tests for Ray unprojection from screen pixel.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <plot3d/Camera.h>
#include <plot3d/Ray.h>

#include <QSize>
#include <cmath>

using namespace lumen::plot3d;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("Ray: center pixel ray points from camera toward target", "[ray]") {
    Camera cam;
    QSize viewport(800, 600);

    // Center pixel.
    QPoint center(400, 300);
    Ray ray = Ray::fromScreenPixel(center, cam, viewport);

    // Ray direction should roughly point from camera position to target.
    QVector3D toTarget = (cam.target() - cam.position()).normalized();
    float dot = QVector3D::dotProduct(ray.direction, toTarget);
    CHECK(dot > 0.9f);
}

TEST_CASE("Ray: ray direction is normalized", "[ray]") {
    Camera cam;
    QSize viewport(800, 600);

    Ray ray = Ray::fromScreenPixel(QPoint(200, 150), cam, viewport);
    CHECK_THAT(static_cast<double>(ray.direction.length()),
               WithinAbs(1.0, 0.001));
}

TEST_CASE("Ray: off-center pixel has different direction than center", "[ray]") {
    Camera cam;
    QSize viewport(800, 600);

    Ray centerRay = Ray::fromScreenPixel(QPoint(400, 300), cam, viewport);
    Ray cornerRay = Ray::fromScreenPixel(QPoint(0, 0), cam, viewport);

    float dot = QVector3D::dotProduct(centerRay.direction, cornerRay.direction);
    // Should be similar but not identical.
    CHECK(dot < 0.999f);
    CHECK(dot > 0.5f);
}

TEST_CASE("Ray: pointAt returns origin at t=0", "[ray]") {
    Ray ray;
    ray.origin = QVector3D(1, 2, 3);
    ray.direction = QVector3D(0, 0, -1);

    QVector3D p = ray.pointAt(0.0);
    CHECK_THAT(static_cast<double>((p - ray.origin).length()),
               WithinAbs(0.0, 1e-5));
}

TEST_CASE("Ray: pointAt returns correct position at t=1", "[ray]") {
    Ray ray;
    ray.origin = QVector3D(0, 0, 0);
    ray.direction = QVector3D(1, 0, 0);

    QVector3D p = ray.pointAt(1.0);
    CHECK_THAT(static_cast<double>(p.x()), WithinAbs(1.0, 1e-5));
    CHECK_THAT(static_cast<double>(p.y()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(p.z()), WithinAbs(0.0, 1e-5));
}

TEST_CASE("Ray: ray from orbit camera center pixel points at target", "[ray]") {
    Camera cam;
    cam.setMode(CameraMode::Orbit);
    cam.handleDrag(QPointF(100.0, 50.0));
    QSize viewport(1024, 768);

    QPoint center(512, 384);
    Ray ray = Ray::fromScreenPixel(center, cam, viewport);

    QVector3D toTarget = (cam.target() - cam.position()).normalized();
    float dot = QVector3D::dotProduct(ray.direction, toTarget);
    CHECK(dot > 0.9f);
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Ray unprojection tests skipped (no OpenGL widgets)", "[ray]") {
    SUCCEED("OpenGL widgets not available — skipping ray unprojection tests");
}

#endif
