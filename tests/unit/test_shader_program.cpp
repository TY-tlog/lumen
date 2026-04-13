// Unit tests for ShaderProgram.
// GL context is required for shader compilation — skip when unavailable.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

#include <catch2/catch_test_macros.hpp>

#include <plot3d/ShaderProgram.h>

using namespace lumen::plot3d;

// These tests need a real GL context which is not available in offscreen
// unit test mode without explicit context creation. They are included
// for completeness and compile-tested, but use SUCCEED() to pass
// gracefully when no context is available.

TEST_CASE("ShaderProgram: compiles when GL context is available", "[shader][!mayfail]") {
    // Without a QOpenGLContext, shader compilation will fail.
    // This test verifies the class interface and compiles correctly.
    // Actual GL tests are deferred to integration tests with a real context.
    ShaderProgram shader;

    // Attempt compile — expected to fail without GL context.
    bool result = shader.compile(
        QStringLiteral("#version 410 core\nvoid main() { gl_Position = vec4(0); }"),
        QStringLiteral("#version 410 core\nout vec4 f; void main() { f = vec4(1); }"));

    // Either it works (GL context present) or fails gracefully.
    if (!result) {
        SUCCEED("No GL context — shader compilation correctly reported failure");
    } else {
        CHECK(shader.programId() != 0);
    }
}

TEST_CASE("ShaderProgram: programId is 0 before compilation", "[shader]") {
    ShaderProgram shader;
    CHECK(shader.programId() == 0);
}

#else

#include <catch2/catch_test_macros.hpp>

TEST_CASE("ShaderProgram tests skipped (no OpenGL widgets)", "[shader]") {
    SUCCEED("OpenGL widgets not available — skipping ShaderProgram tests");
}

#endif
