// Unit tests for PbrMaterial and PBR shader selection in Renderer3D.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <plot3d/PbrMaterial.h>
#include <plot3d/PlotItem3D.h>
#include <plot3d/Renderer3D.h>
#include <plot3d/Scatter3D.h>

#include <QJsonObject>

using namespace lumen::plot3d;
using namespace lumen::data;
using Catch::Matchers::WithinAbs;

TEST_CASE("PbrMaterial: JSON roundtrip", "[pbr_material]") {
    PbrMaterial mat;
    mat.baseColor = QColor(100, 150, 200);
    mat.metallic = 0.8f;
    mat.roughness = 0.3f;
    mat.ior = 2.0f;
    mat.emissive = QColor(10, 20, 30);

    QJsonObject json = mat.toJson();
    PbrMaterial mat2 = PbrMaterial::fromJson(json);

    CHECK(mat2.baseColor == QColor(100, 150, 200));
    CHECK_THAT(static_cast<double>(mat2.metallic), WithinAbs(0.8, 1e-5));
    CHECK_THAT(static_cast<double>(mat2.roughness), WithinAbs(0.3, 1e-5));
    CHECK_THAT(static_cast<double>(mat2.ior), WithinAbs(2.0, 1e-5));
    CHECK(mat2.emissive == QColor(10, 20, 30));
}

TEST_CASE("PbrMaterial: default values", "[pbr_material]") {
    PbrMaterial mat;

    CHECK(mat.baseColor == QColor(200, 200, 200));
    CHECK_THAT(static_cast<double>(mat.metallic), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(mat.roughness), WithinAbs(0.5, 1e-5));
    CHECK_THAT(static_cast<double>(mat.ior), WithinAbs(1.5, 1e-5));
    CHECK(mat.emissive == QColor(0, 0, 0));
}

TEST_CASE("PbrMaterial: equality operator", "[pbr_material]") {
    PbrMaterial a;
    PbrMaterial b;
    CHECK(a == b);

    b.metallic = 1.0f;
    CHECK(a != b);
}

TEST_CASE("PbrMaterial: fromJson uses defaults for missing keys", "[pbr_material]") {
    QJsonObject emptyObj;
    PbrMaterial mat = PbrMaterial::fromJson(emptyObj);

    CHECK(mat.baseColor == QColor(200, 200, 200));
    CHECK_THAT(static_cast<double>(mat.metallic), WithinAbs(0.0, 1e-5));
    CHECK_THAT(static_cast<double>(mat.roughness), WithinAbs(0.5, 1e-5));
}

TEST_CASE("PlotItem3D: PBR material getter/setter", "[pbr_material]") {
    auto xDs = std::make_shared<Rank1Dataset>(
        "x", Unit::dimensionless(), std::vector<double>{1.0});
    auto yDs = std::make_shared<Rank1Dataset>(
        "y", Unit::dimensionless(), std::vector<double>{2.0});
    auto zDs = std::make_shared<Rank1Dataset>(
        "z", Unit::dimensionless(), std::vector<double>{3.0});

    Scatter3D scatter(xDs, yDs, zDs);

    // Initially no PBR material.
    CHECK_FALSE(scatter.hasPbrMaterial());

    // Set PBR material.
    PbrMaterial mat;
    mat.metallic = 0.9f;
    mat.roughness = 0.1f;
    scatter.setPbrMaterial(mat);

    CHECK(scatter.hasPbrMaterial());
    REQUIRE(scatter.pbrMaterial().has_value());
    CHECK_THAT(static_cast<double>(scatter.pbrMaterial()->metallic), WithinAbs(0.9, 1e-5));

    // Clear PBR material.
    scatter.clearPbrMaterial();
    CHECK_FALSE(scatter.hasPbrMaterial());
}

TEST_CASE("Renderer3D: shader selection based on PBR material", "[pbr_material]") {
    auto xDs = std::make_shared<Rank1Dataset>(
        "x", Unit::dimensionless(), std::vector<double>{1.0});
    auto yDs = std::make_shared<Rank1Dataset>(
        "y", Unit::dimensionless(), std::vector<double>{2.0});
    auto zDs = std::make_shared<Rank1Dataset>(
        "z", Unit::dimensionless(), std::vector<double>{3.0});

    Scatter3D scatter(xDs, yDs, zDs);
    Renderer3D renderer;

    // Before initialization, both should return the phong shader (uninitialized).
    // After init, items without PBR material should use Phong.
    // We can't fully initialize (needs GL context), but we can test the logic
    // by checking shaderForItem behavior.

    // Without PBR material -> should return phong (the default).
    ShaderProgram& s1 = renderer.shaderForItem(scatter);

    // With PBR material but PBR not initialized -> should still return phong.
    PbrMaterial mat;
    scatter.setPbrMaterial(mat);
    ShaderProgram& s2 = renderer.shaderForItem(scatter);

    // Both should reference the same shader since PBR isn't initialized.
    CHECK(&s1 == &s2);
}

TEST_CASE("PbrMaterial: JSON roundtrip preserves all fields", "[pbr_material]") {
    PbrMaterial original;
    original.baseColor = QColor(42, 128, 255);
    original.metallic = 0.42f;
    original.roughness = 0.73f;
    original.ior = 1.8f;
    original.emissive = QColor(5, 10, 15);

    QJsonObject json = original.toJson();
    PbrMaterial restored = PbrMaterial::fromJson(json);

    CHECK(restored == original);
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("PbrMaterial tests skipped (no OpenGL widgets)", "[pbr_material]") {
    SUCCEED("OpenGL widgets not available -- skipping PbrMaterial tests");
}

#endif
