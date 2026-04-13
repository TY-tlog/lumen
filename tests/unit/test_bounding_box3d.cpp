// Unit tests for BoundingBox3D.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <plot3d/BoundingBox3D.h>

using namespace lumen::plot3d;
using Catch::Matchers::WithinAbs;

TEST_CASE("BoundingBox3D: default box is valid (min==max==0)", "[bbox3d]") {
    BoundingBox3D box;
    CHECK(box.isValid());
}

TEST_CASE("BoundingBox3D: valid when min <= max", "[bbox3d]") {
    BoundingBox3D box{QVector3D(-1, -1, -1), QVector3D(1, 1, 1)};
    CHECK(box.isValid());
}

TEST_CASE("BoundingBox3D: invalid when min > max in any axis", "[bbox3d]") {
    BoundingBox3D box{QVector3D(1, -1, -1), QVector3D(-1, 1, 1)};
    CHECK_FALSE(box.isValid());

    BoundingBox3D boxY{QVector3D(0, 5, 0), QVector3D(1, 1, 1)};
    CHECK_FALSE(boxY.isValid());

    BoundingBox3D boxZ{QVector3D(0, 0, 3), QVector3D(1, 1, 1)};
    CHECK_FALSE(boxZ.isValid());
}

TEST_CASE("BoundingBox3D: center of symmetric box is origin", "[bbox3d]") {
    BoundingBox3D box{QVector3D(-2, -3, -4), QVector3D(2, 3, 4)};
    QVector3D c = box.center();
    CHECK_THAT(static_cast<double>(c.x()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(c.y()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(c.z()), WithinAbs(0.0, 1e-5));
}

TEST_CASE("BoundingBox3D: center of offset box", "[bbox3d]") {
    BoundingBox3D box{QVector3D(1, 2, 3), QVector3D(5, 6, 7)};
    QVector3D c = box.center();
    CHECK_THAT(static_cast<double>(c.x()), WithinAbs(3.0, 1e-5));
    CHECK_THAT(static_cast<double>(c.y()), WithinAbs(4.0, 1e-5));
    CHECK_THAT(static_cast<double>(c.z()), WithinAbs(5.0, 1e-5));
}

TEST_CASE("BoundingBox3D: size is correct", "[bbox3d]") {
    BoundingBox3D box{QVector3D(1, 2, 3), QVector3D(4, 6, 10)};
    QVector3D s = box.size();
    CHECK_THAT(static_cast<double>(s.x()), WithinAbs(3.0, 1e-5));
    CHECK_THAT(static_cast<double>(s.y()), WithinAbs(4.0, 1e-5));
    CHECK_THAT(static_cast<double>(s.z()), WithinAbs(7.0, 1e-5));
}

TEST_CASE("BoundingBox3D: united with overlapping box", "[bbox3d]") {
    BoundingBox3D a{QVector3D(0, 0, 0), QVector3D(2, 2, 2)};
    BoundingBox3D b{QVector3D(1, 1, 1), QVector3D(3, 3, 3)};

    BoundingBox3D u = a.united(b);
    CHECK_THAT(static_cast<double>(u.min.x()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(u.min.y()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(u.min.z()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(u.max.x()), WithinAbs(3.0, 1e-5));
    CHECK_THAT(static_cast<double>(u.max.y()), WithinAbs(3.0, 1e-5));
    CHECK_THAT(static_cast<double>(u.max.z()), WithinAbs(3.0, 1e-5));
}

TEST_CASE("BoundingBox3D: united with disjoint box", "[bbox3d]") {
    BoundingBox3D a{QVector3D(-5, -5, -5), QVector3D(-3, -3, -3)};
    BoundingBox3D b{QVector3D(3, 3, 3), QVector3D(5, 5, 5)};

    BoundingBox3D u = a.united(b);
    CHECK_THAT(static_cast<double>(u.min.x()), WithinAbs(-5.0, 1e-5));
    CHECK_THAT(static_cast<double>(u.max.x()), WithinAbs(5.0, 1e-5));
}

TEST_CASE("BoundingBox3D: united with invalid box returns other", "[bbox3d]") {
    BoundingBox3D invalid{QVector3D(5, 0, 0), QVector3D(0, 0, 0)};
    BoundingBox3D valid{QVector3D(1, 1, 1), QVector3D(2, 2, 2)};

    BoundingBox3D u = invalid.united(valid);
    CHECK_THAT(static_cast<double>(u.min.x()), WithinAbs(1.0, 1e-5));
    CHECK_THAT(static_cast<double>(u.max.x()), WithinAbs(2.0, 1e-5));
}

TEST_CASE("BoundingBox3D: united of valid with invalid returns valid", "[bbox3d]") {
    BoundingBox3D valid{QVector3D(1, 1, 1), QVector3D(2, 2, 2)};
    BoundingBox3D invalid{QVector3D(5, 0, 0), QVector3D(0, 0, 0)};

    BoundingBox3D u = valid.united(invalid);
    CHECK_THAT(static_cast<double>(u.min.x()), WithinAbs(1.0, 1e-5));
    CHECK_THAT(static_cast<double>(u.max.x()), WithinAbs(2.0, 1e-5));
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("BoundingBox3D tests skipped (no OpenGL widgets)", "[bbox3d]") {
    SUCCEED("OpenGL widgets not available — skipping BoundingBox3D tests");
}

#endif
