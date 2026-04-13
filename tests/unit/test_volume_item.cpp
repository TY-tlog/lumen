// Unit tests for VolumeItem.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Unit.h>
#include <data/Volume3D.h>
#include <plot3d/Ray.h>
#include <plot3d/VolumeItem.h>

#include <memory>

using namespace lumen::plot3d;
using namespace lumen::data;
using Catch::Matchers::WithinAbs;

namespace {

std::shared_ptr<Volume3D> makeTestVolume()
{
    Dimension dimX{QStringLiteral("x"), Unit::dimensionless(), 4,
                   CoordinateArray(0.0, 1.0, 4)};
    Dimension dimY{QStringLiteral("y"), Unit::dimensionless(), 4,
                   CoordinateArray(0.0, 1.0, 4)};
    Dimension dimZ{QStringLiteral("z"), Unit::dimensionless(), 4,
                   CoordinateArray(0.0, 1.0, 4)};

    std::vector<double> data(64, 0.5);
    return std::make_shared<Volume3D>(QStringLiteral("test_vol"),
                                     Unit::dimensionless(),
                                     std::move(dimX), std::move(dimY),
                                     std::move(dimZ), std::move(data));
}

}  // namespace

TEST_CASE("VolumeItem: type is Volume", "[volume_item]") {
    auto vol = makeTestVolume();
    VolumeItem item(vol, "test");
    CHECK(item.type() == PlotItem3D::Type::Volume);
}

TEST_CASE("VolumeItem: dataBounds from volume dimensions", "[volume_item]") {
    auto vol = makeTestVolume();
    VolumeItem item(vol);

    auto bounds = item.dataBounds();
    CHECK(bounds.isValid());
    CHECK_THAT(static_cast<double>(bounds.min.x()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.max.x()), WithinAbs(3.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.min.y()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.max.y()), WithinAbs(3.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.min.z()), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.max.z()), WithinAbs(3.0, 1e-5));
}

TEST_CASE("VolumeItem: hitTestRay detects AABB", "[volume_item]") {
    auto vol = makeTestVolume();
    VolumeItem item(vol);

    // Ray through the center of the volume.
    Ray ray{QVector3D(1.5f, 1.5f, 10.0f), QVector3D(0, 0, -1)};
    auto hit = item.hitTestRay(ray, 100.0);
    REQUIRE(hit.has_value());
    // Should hit near z=3 (top face of the bounding box).
    CHECK_THAT(hit->distance, WithinAbs(7.0, 0.1));
}

TEST_CASE("VolumeItem: hitTestRay misses volume", "[volume_item]") {
    auto vol = makeTestVolume();
    VolumeItem item(vol);

    // Ray that completely misses.
    Ray ray{QVector3D(100.0f, 100.0f, 10.0f), QVector3D(0, 0, -1)};
    auto hit = item.hitTestRay(ray, 100.0);
    CHECK_FALSE(hit.has_value());
}

TEST_CASE("VolumeItem: sampling parameters", "[volume_item]") {
    auto vol = makeTestVolume();
    VolumeItem item(vol);

    CHECK_THAT(static_cast<double>(item.sampleStep()), WithinAbs(0.01, 1e-5));
    CHECK(item.maxSamples() == 256);

    item.setSampleStep(0.05f);
    item.setMaxSamples(512);

    CHECK_THAT(static_cast<double>(item.sampleStep()), WithinAbs(0.05, 1e-5));
    CHECK(item.maxSamples() == 512);
}

TEST_CASE("VolumeItem: default transfer function", "[volume_item]") {
    auto vol = makeTestVolume();
    VolumeItem item(vol);

    const auto& tf = item.transferFunction();
    CHECK(tf.controlPoints().size() == 2);

    // Generate a LUT to check it works.
    QImage lut = tf.toLUT(64);
    CHECK(lut.width() == 64);
}

TEST_CASE("VolumeItem: visibility", "[volume_item]") {
    auto vol = makeTestVolume();
    VolumeItem item(vol);

    CHECK(item.isVisible());
    item.setVisible(false);
    CHECK_FALSE(item.isVisible());
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("VolumeItem tests skipped (no OpenGL widgets)", "[volume_item]") {
    SUCCEED("OpenGL widgets not available -- skipping VolumeItem tests");
}

#endif
