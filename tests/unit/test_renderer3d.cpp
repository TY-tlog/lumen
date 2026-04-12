// Unit tests for Renderer3D.
// GL context is required — skip when unavailable.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>

#include <plot3d/Camera.h>
#include <plot3d/Renderer3D.h>
#include <plot3d/Scene3D.h>

using namespace lumen::plot3d;

TEST_CASE("Renderer3D: not initialized by default", "[renderer3d]") {
    Renderer3D renderer;
    CHECK_FALSE(renderer.isInitialized());
}

TEST_CASE("Renderer3D: initialize without GL context fails gracefully", "[renderer3d][!mayfail]") {
    Renderer3D renderer;

    // Without a QOpenGLContext, initialization will fail.
    bool result = renderer.initialize();

    if (!result) {
        SUCCEED("No GL context — Renderer3D initialization correctly reported failure");
        CHECK_FALSE(renderer.isInitialized());
    } else {
        CHECK(renderer.isInitialized());
    }
}

TEST_CASE("Renderer3D: render on uninitialized renderer is a no-op", "[renderer3d]") {
    Renderer3D renderer;
    Scene3D scene;
    Camera camera;

    // Should not crash.
    renderer.render(scene, camera, QSize(800, 600));
    SUCCEED("Render on uninitialized renderer did not crash");
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Renderer3D tests skipped (no OpenGL widgets)", "[renderer3d]") {
    SUCCEED("OpenGL widgets not available — skipping Renderer3D tests");
}

#endif
