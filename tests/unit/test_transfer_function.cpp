// Unit tests for TransferFunction.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <plot3d/TransferFunction.h>

#include <QJsonObject>

using namespace lumen::plot3d;
using Catch::Matchers::WithinAbs;

TEST_CASE("TransferFunction: LUT has correct size", "[transfer_function]") {
    TransferFunction tf;
    tf.addControlPoint(0.0, QColor(0, 0, 0), 0.0);
    tf.addControlPoint(1.0, QColor(255, 255, 255), 1.0);

    QImage lut = tf.toLUT(128);
    CHECK(lut.width() == 128);
    CHECK(lut.height() == 1);
}

TEST_CASE("TransferFunction: LUT endpoints match control points", "[transfer_function]") {
    TransferFunction tf;
    tf.addControlPoint(0.0, QColor(255, 0, 0), 0.0);
    tf.addControlPoint(1.0, QColor(0, 0, 255), 1.0);

    QImage lut = tf.toLUT(256);

    // First pixel should be red, transparent.
    QColor first = lut.pixelColor(0, 0);
    CHECK(first.red() == 255);
    CHECK(first.green() == 0);
    CHECK(first.blue() == 0);
    CHECK(first.alpha() == 0);

    // Last pixel should be blue, opaque.
    QColor last = lut.pixelColor(255, 0);
    CHECK(last.red() == 0);
    CHECK(last.green() == 0);
    CHECK(last.blue() == 255);
    CHECK(last.alpha() == 255);
}

TEST_CASE("TransferFunction: sample interpolates correctly", "[transfer_function]") {
    TransferFunction tf;
    tf.addControlPoint(0.0, QColor(0, 0, 0), 0.0);
    tf.addControlPoint(1.0, QColor(200, 100, 50), 1.0);

    QColor mid = tf.sample(0.5);
    CHECK_THAT(static_cast<double>(mid.red()), WithinAbs(100.0, 2.0));
    CHECK_THAT(static_cast<double>(mid.green()), WithinAbs(50.0, 2.0));
    CHECK_THAT(static_cast<double>(mid.blue()), WithinAbs(25.0, 2.0));
    CHECK_THAT(static_cast<double>(mid.alphaF()), WithinAbs(0.5, 0.02));
}

TEST_CASE("TransferFunction: JSON roundtrip", "[transfer_function]") {
    TransferFunction tf;
    tf.addControlPoint(0.0, QColor(255, 0, 0), 0.0);
    tf.addControlPoint(0.5, QColor(0, 255, 0), 0.5);
    tf.addControlPoint(1.0, QColor(0, 0, 255), 1.0);

    QJsonObject json = tf.toJson();
    TransferFunction tf2 = TransferFunction::fromJson(json);

    CHECK(tf2.controlPoints().size() == 3);
    CHECK_THAT(tf2.controlPoints()[0].value, WithinAbs(0.0, 1e-10));
    CHECK_THAT(tf2.controlPoints()[1].value, WithinAbs(0.5, 1e-10));
    CHECK_THAT(tf2.controlPoints()[2].value, WithinAbs(1.0, 1e-10));

    CHECK(tf2.controlPoints()[0].color.red() == 255);
    CHECK(tf2.controlPoints()[1].color.green() == 255);
    CHECK(tf2.controlPoints()[2].color.blue() == 255);
}

TEST_CASE("TransferFunction: clamping at boundaries", "[transfer_function]") {
    TransferFunction tf;
    tf.addControlPoint(0.2, QColor(100, 100, 100), 0.5);
    tf.addControlPoint(0.8, QColor(200, 200, 200), 0.8);

    // Values outside the control point range should clamp.
    QColor below = tf.sample(0.0);
    CHECK(below.red() == 100);

    QColor above = tf.sample(1.0);
    CHECK(above.red() == 200);
}

TEST_CASE("TransferFunction: empty returns white", "[transfer_function]") {
    TransferFunction tf;
    QColor c = tf.sample(0.5);
    CHECK(c.red() == 255);
    CHECK(c.green() == 255);
    CHECK(c.blue() == 255);
}

TEST_CASE("TransferFunction: control points are sorted", "[transfer_function]") {
    TransferFunction tf;
    tf.addControlPoint(0.8, QColor(0, 0, 255), 1.0);
    tf.addControlPoint(0.2, QColor(255, 0, 0), 0.0);
    tf.addControlPoint(0.5, QColor(0, 255, 0), 0.5);

    const auto& pts = tf.controlPoints();
    REQUIRE(pts.size() == 3);
    CHECK_THAT(pts[0].value, WithinAbs(0.2, 1e-10));
    CHECK_THAT(pts[1].value, WithinAbs(0.5, 1e-10));
    CHECK_THAT(pts[2].value, WithinAbs(0.8, 1e-10));
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("TransferFunction tests skipped (no OpenGL widgets)", "[transfer_function]") {
    SUCCEED("OpenGL widgets not available -- skipping TransferFunction tests");
}

#endif
