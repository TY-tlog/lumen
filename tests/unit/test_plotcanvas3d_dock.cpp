// PlotCanvas3DDock unit tests.
// These test basic QDockWidget construction without requiring OpenGL.

#include <catch2/catch_test_macros.hpp>

#include <QDockWidget>
#include <QString>

// We test the dock's QDockWidget properties without constructing the actual
// PlotCanvas3D (which requires an OpenGL context). Instead, test the dock
// source file directly.

#ifdef LUMEN_HAS_OPENGL_WIDGETS

// On systems with OpenGL, we can only test that the types compile.
// Full construction requires a display, which may not be available.

#include "ui/PlotCanvas3DDock.h"

TEST_CASE("PlotCanvas3DDock: type compiles and has correct base",
          "[ui][plotcanvas3d_dock]") {
    // Verify the type inherits QDockWidget (compile-time check).
    static_assert(std::is_base_of_v<QDockWidget, lumen::ui::PlotCanvas3DDock>);
    SUCCEED("PlotCanvas3DDock compiles with QDockWidget base");
}

#else

TEST_CASE("PlotCanvas3DDock tests skipped (no OpenGL widgets)",
          "[plotcanvas3d_dock]") {
    SUCCEED("OpenGL widgets not available -- skipping PlotCanvas3DDock tests");
}

#endif
