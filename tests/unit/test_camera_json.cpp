// Unit tests for Camera JSON persistence.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <plot3d/Camera.h>

#include <QJsonDocument>
#include <QJsonObject>

using namespace lumen::plot3d;
using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinAbs;

TEST_CASE("Camera: JSON roundtrip preserves trackball state", "[camera][json]") {
    Camera original;
    original.setTarget(QVector3D(1.0f, 2.0f, 3.0f));
    original.setFov(60.0f);
    original.setNearFar(0.5f, 500.0f);
    original.handleDrag(QPointF(100.0, 50.0));

    QJsonObject json = original.toJson();

    Camera restored;
    restored.fromJson(json);

    CHECK(restored.mode() == CameraMode::Trackball);
    CHECK_THAT(static_cast<double>(restored.target().x()), WithinAbs(1.0, 1e-4));
    CHECK_THAT(static_cast<double>(restored.target().y()), WithinAbs(2.0, 1e-4));
    CHECK_THAT(static_cast<double>(restored.target().z()), WithinAbs(3.0, 1e-4));
    CHECK_THAT(static_cast<double>(restored.fov()), WithinRel(60.0, 0.01));
    CHECK_THAT(static_cast<double>(restored.nearPlane()), WithinRel(0.5, 0.01));
    CHECK_THAT(static_cast<double>(restored.farPlane()), WithinRel(500.0, 0.01));

    // Positions should match after roundtrip.
    QVector3D origPos = original.position();
    QVector3D restPos = restored.position();
    CHECK_THAT(static_cast<double>((origPos - restPos).length()),
               WithinAbs(0.0, 0.1));
}

TEST_CASE("Camera: JSON roundtrip preserves orbit state", "[camera][json]") {
    Camera original;
    original.setMode(CameraMode::Orbit);
    original.setTarget(QVector3D(-1.0f, 0.0f, 2.0f));
    original.handleDrag(QPointF(200.0, 100.0));

    QJsonObject json = original.toJson();

    Camera restored;
    restored.fromJson(json);

    CHECK(restored.mode() == CameraMode::Orbit);

    QVector3D origPos = original.position();
    QVector3D restPos = restored.position();
    CHECK_THAT(static_cast<double>((origPos - restPos).length()),
               WithinAbs(0.0, 0.1));
}

TEST_CASE("Camera: JSON includes mode string", "[camera][json]") {
    Camera cam;
    QJsonObject json = cam.toJson();
    CHECK(json[QStringLiteral("mode")].toString() == QStringLiteral("trackball"));

    cam.setMode(CameraMode::Orbit);
    json = cam.toJson();
    CHECK(json[QStringLiteral("mode")].toString() == QStringLiteral("orbit"));
}

TEST_CASE("Camera: JSON includes all expected keys", "[camera][json]") {
    Camera cam;
    QJsonObject json = cam.toJson();

    CHECK(json.contains(QStringLiteral("mode")));
    CHECK(json.contains(QStringLiteral("target")));
    CHECK(json.contains(QStringLiteral("distance")));
    CHECK(json.contains(QStringLiteral("fov")));
    CHECK(json.contains(QStringLiteral("near")));
    CHECK(json.contains(QStringLiteral("far")));
    CHECK(json.contains(QStringLiteral("orientation")));
    CHECK(json.contains(QStringLiteral("azimuth")));
    CHECK(json.contains(QStringLiteral("elevation")));
}

TEST_CASE("Camera: fromJson with empty object uses defaults", "[camera][json]") {
    Camera cam;
    cam.fromJson(QJsonObject{});

    CHECK(cam.mode() == CameraMode::Trackball);
    CHECK_THAT(static_cast<double>(cam.distance()), WithinRel(5.0, 0.01));
    CHECK_THAT(static_cast<double>(cam.fov()), WithinRel(45.0, 0.01));
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Camera JSON tests skipped (no OpenGL widgets)", "[camera][json]") {
    SUCCEED("OpenGL widgets not available — skipping camera JSON tests");
}

#endif
