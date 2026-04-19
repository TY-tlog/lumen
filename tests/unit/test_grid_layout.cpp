#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <dashboard/GridLayout.h>

using namespace lumen::dashboard;
using Catch::Matchers::WithinAbs;
namespace {
PanelConfig pc(int row = 0, int col = 0, int rs = 1, int cs = 1)
{
    PanelConfig c;
    c.row = row;
    c.col = col;
    c.rowSpan = rs;
    c.colSpan = cs;
    return c;
}
}  // namespace


TEST_CASE("GridLayout: default is 1x1", "[dashboard][layout]")
{
    GridLayout gl;
    REQUIRE(gl.rows() == 1);
    REQUIRE(gl.cols() == 1);
}

TEST_CASE("GridLayout: 1x1 panel fills entire area", "[dashboard][layout]")
{
    GridLayout gl;
    PanelConfig p = pc(0, 0, 1, 1);
    QRectF r = gl.cellRect(p, QSizeF(800, 600));
    REQUIRE_THAT(r.x(), WithinAbs(0.0, 0.1));
    REQUIRE_THAT(r.y(), WithinAbs(0.0, 0.1));
    REQUIRE_THAT(r.width(), WithinAbs(800.0, 0.1));
    REQUIRE_THAT(r.height(), WithinAbs(600.0, 0.1));
}

TEST_CASE("GridLayout: 2x2 produces 4 equal cells", "[dashboard][layout]")
{
    GridLayout gl;
    gl.setGridSize(2, 2);
    gl.setSpacing(0);

    QSizeF area(800, 600);
    PanelConfig p00 = pc(0, 0);
    PanelConfig p01 = pc(0, 1);
    PanelConfig p10 = pc(1, 0);
    PanelConfig p11 = pc(1, 1);

    QRectF r00 = gl.cellRect(p00, area);
    QRectF r01 = gl.cellRect(p01, area);
    QRectF r10 = gl.cellRect(p10, area);
    QRectF r11 = gl.cellRect(p11, area);

    REQUIRE_THAT(r00.width(), WithinAbs(400.0, 0.1));
    REQUIRE_THAT(r00.height(), WithinAbs(300.0, 0.1));
    REQUIRE_THAT(r01.x(), WithinAbs(400.0, 0.1));
    REQUIRE_THAT(r10.y(), WithinAbs(300.0, 0.1));
    REQUIRE_THAT(r11.x(), WithinAbs(400.0, 0.1));
    REQUIRE_THAT(r11.y(), WithinAbs(300.0, 0.1));
}

TEST_CASE("GridLayout: spacing reduces cell size", "[dashboard][layout]")
{
    GridLayout gl;
    gl.setGridSize(2, 2);
    gl.setSpacing(8);

    QSizeF area(808, 608);
    PanelConfig p = pc(0, 0);
    QRectF r = gl.cellRect(p, area);

    REQUIRE_THAT(r.width(), WithinAbs(400.0, 0.1));
    REQUIRE_THAT(r.height(), WithinAbs(300.0, 0.1));
}

TEST_CASE("GridLayout: cell spanning doubles width", "[dashboard][layout]")
{
    GridLayout gl;
    gl.setGridSize(2, 2);
    gl.setSpacing(0);

    PanelConfig wide = pc(0, 0, 1, 2);
    QRectF r = gl.cellRect(wide, QSizeF(800, 600));
    REQUIRE_THAT(r.width(), WithinAbs(800.0, 0.1));
    REQUIRE_THAT(r.height(), WithinAbs(300.0, 0.1));
}

TEST_CASE("GridLayout: row spanning doubles height", "[dashboard][layout]")
{
    GridLayout gl;
    gl.setGridSize(2, 2);
    gl.setSpacing(0);

    PanelConfig tall = pc(0, 0, 2, 1);
    QRectF r = gl.cellRect(tall, QSizeF(800, 600));
    REQUIRE_THAT(r.width(), WithinAbs(400.0, 0.1));
    REQUIRE_THAT(r.height(), WithinAbs(600.0, 0.1));
}

TEST_CASE("GridLayout: clamps row/col to grid bounds", "[dashboard][layout]")
{
    GridLayout gl;
    gl.setGridSize(2, 2);
    gl.setSpacing(0);

    PanelConfig oob = pc(5, 5);
    QRectF r = gl.cellRect(oob, QSizeF(800, 600));
    REQUIRE(r.x() >= 0);
    REQUIRE(r.y() >= 0);
}

TEST_CASE("GridLayout: clamps span to remaining cells", "[dashboard][layout]")
{
    GridLayout gl;
    gl.setGridSize(2, 2);
    gl.setSpacing(0);

    PanelConfig big = pc(0, 1, 1, 5);
    QRectF r = gl.cellRect(big, QSizeF(800, 600));
    REQUIRE_THAT(r.width(), WithinAbs(400.0, 0.1));
}

TEST_CASE("GridLayout: allRects returns one rect per panel", "[dashboard][layout]")
{
    GridLayout gl;
    gl.setGridSize(2, 2);

    std::vector<PanelConfig> panels = {pc(0, 0), pc(0, 1), pc(1, 0)};
    auto rects = gl.allRects(panels, QSizeF(800, 600));
    REQUIRE(rects.size() == 3);
}

TEST_CASE("GridLayout: 3x2 grid with spacing", "[dashboard][layout]")
{
    GridLayout gl;
    gl.setGridSize(3, 2);
    gl.setSpacing(10);

    QSizeF area(410, 620);
    PanelConfig p = pc(2, 1);
    QRectF r = gl.cellRect(p, area);

    REQUIRE(r.x() > 200.0);
    REQUIRE(r.y() > 400.0);
    REQUIRE(r.width() > 0);
    REQUIRE(r.height() > 0);
}

TEST_CASE("GridLayout: setGridSize clamps values", "[dashboard][layout]")
{
    GridLayout gl;
    gl.setGridSize(0, -1);
    REQUIRE(gl.rows() == 1);
    REQUIRE(gl.cols() == 1);

    gl.setGridSize(100, 100);
    REQUIRE(gl.rows() == 8);
    REQUIRE(gl.cols() == 8);
}
