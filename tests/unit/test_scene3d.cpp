// Unit tests for Scene3D.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <plot3d/BoundingBox3D.h>
#include <plot3d/Light.h>
#include <plot3d/PlotItem3D.h>
#include <plot3d/Scene3D.h>

#include <memory>

using namespace lumen::plot3d;
using Catch::Matchers::WithinAbs;

namespace {

/// Minimal concrete PlotItem3D for testing.
class DummyItem3D : public PlotItem3D {
public:
    DummyItem3D(QString name, BoundingBox3D bounds, bool visible = true)
        : name_(std::move(name)), bounds_(bounds), visible_(visible)
    {
    }

    Type type() const override { return Type::Scatter3D; }
    QString name() const override { return name_; }
    BoundingBox3D dataBounds() const override { return bounds_; }
    void render(ShaderProgram& /*shader*/, const RenderContext& /*ctx*/) override {}
    std::optional<HitResult3D> hitTestRay(const Ray& /*ray*/,
                                           double /*maxDist*/) const override
    {
        return std::nullopt;
    }
    bool isVisible() const override { return visible_; }

private:
    QString name_;
    BoundingBox3D bounds_;
    bool visible_;
};

}  // namespace

TEST_CASE("Scene3D: starts empty", "[scene3d]") {
    Scene3D scene;
    CHECK(scene.itemCount() == 0);
    CHECK(scene.items().empty());
}

TEST_CASE("Scene3D: add items increases count", "[scene3d]") {
    Scene3D scene;
    BoundingBox3D bounds{QVector3D(0, 0, 0), QVector3D(1, 1, 1)};

    scene.addItem(std::make_unique<DummyItem3D>("item1", bounds));
    CHECK(scene.itemCount() == 1);

    scene.addItem(std::make_unique<DummyItem3D>("item2", bounds));
    CHECK(scene.itemCount() == 2);
}

TEST_CASE("Scene3D: itemAt returns correct item", "[scene3d]") {
    Scene3D scene;
    BoundingBox3D bounds{QVector3D(0, 0, 0), QVector3D(1, 1, 1)};

    scene.addItem(std::make_unique<DummyItem3D>("first", bounds));
    scene.addItem(std::make_unique<DummyItem3D>("second", bounds));

    CHECK(scene.itemAt(0)->name() == QStringLiteral("first"));
    CHECK(scene.itemAt(1)->name() == QStringLiteral("second"));
    CHECK(scene.itemAt(2) == nullptr);
    CHECK(scene.itemAt(-1) == nullptr);
}

TEST_CASE("Scene3D: removeItem removes correct item", "[scene3d]") {
    Scene3D scene;
    BoundingBox3D bounds{QVector3D(0, 0, 0), QVector3D(1, 1, 1)};

    scene.addItem(std::make_unique<DummyItem3D>("a", bounds));
    scene.addItem(std::make_unique<DummyItem3D>("b", bounds));
    scene.addItem(std::make_unique<DummyItem3D>("c", bounds));

    scene.removeItem(1);
    CHECK(scene.itemCount() == 2);
    CHECK(scene.itemAt(0)->name() == QStringLiteral("a"));
    CHECK(scene.itemAt(1)->name() == QStringLiteral("c"));
}

TEST_CASE("Scene3D: clearItems removes all", "[scene3d]") {
    Scene3D scene;
    BoundingBox3D bounds{QVector3D(0, 0, 0), QVector3D(1, 1, 1)};

    scene.addItem(std::make_unique<DummyItem3D>("a", bounds));
    scene.addItem(std::make_unique<DummyItem3D>("b", bounds));
    scene.clearItems();

    CHECK(scene.itemCount() == 0);
}

TEST_CASE("Scene3D: sceneBounds is union of visible items", "[scene3d]") {
    Scene3D scene;

    scene.addItem(std::make_unique<DummyItem3D>(
        "a", BoundingBox3D{QVector3D(0, 0, 0), QVector3D(1, 1, 1)}));
    scene.addItem(std::make_unique<DummyItem3D>(
        "b", BoundingBox3D{QVector3D(-1, -1, -1), QVector3D(0.5f, 0.5f, 0.5f)}));

    BoundingBox3D bounds = scene.sceneBounds();
    CHECK(bounds.isValid());
    CHECK_THAT(static_cast<double>(bounds.min.x()), WithinAbs(-1.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.min.y()), WithinAbs(-1.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.min.z()), WithinAbs(-1.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.max.x()), WithinAbs(1.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.max.y()), WithinAbs(1.0, 1e-5));
    CHECK_THAT(static_cast<double>(bounds.max.z()), WithinAbs(1.0, 1e-5));
}

TEST_CASE("Scene3D: sceneBounds excludes invisible items", "[scene3d]") {
    Scene3D scene;

    scene.addItem(std::make_unique<DummyItem3D>(
        "visible", BoundingBox3D{QVector3D(0, 0, 0), QVector3D(1, 1, 1)}));
    scene.addItem(std::make_unique<DummyItem3D>(
        "hidden", BoundingBox3D{QVector3D(-10, -10, -10), QVector3D(10, 10, 10)},
        false));

    BoundingBox3D bounds = scene.sceneBounds();
    CHECK_THAT(static_cast<double>(bounds.max.x()), WithinAbs(1.0, 1e-5));
}

TEST_CASE("Scene3D: addDefaultLights adds directional + ambient", "[scene3d]") {
    Scene3D scene;
    scene.addDefaultLights();

    CHECK(scene.lights().size() == 2);

    bool hasDirectional = false;
    bool hasAmbient = false;
    for (const auto& light : scene.lights()) {
        if (light.type == LightType::Directional) hasDirectional = true;
        if (light.type == LightType::Ambient) hasAmbient = true;
    }
    CHECK(hasDirectional);
    CHECK(hasAmbient);
}

TEST_CASE("Scene3D: removeItem with invalid index does nothing", "[scene3d]") {
    Scene3D scene;
    BoundingBox3D bounds{QVector3D(0, 0, 0), QVector3D(1, 1, 1)};
    scene.addItem(std::make_unique<DummyItem3D>("a", bounds));

    scene.removeItem(-1);
    scene.removeItem(5);
    CHECK(scene.itemCount() == 1);
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Scene3D tests skipped (no OpenGL widgets)", "[scene3d]") {
    SUCCEED("OpenGL widgets not available — skipping Scene3D tests");
}

#endif
