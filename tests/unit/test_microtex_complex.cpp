#include <catch2/catch_test_macros.hpp>

#include <export/MathRenderer.h>

using lumen::exp::MathRenderer;

TEST_CASE("MathRenderer: fraction renders", "[math_renderer]") {
    QImage img = MathRenderer::render(QStringLiteral("\\frac{a}{b}"), 14.0);
    CHECK_FALSE(img.isNull());
    CHECK(img.width() > 0);
}

TEST_CASE("MathRenderer: integral renders", "[math_renderer]") {
    QImage img = MathRenderer::render(QStringLiteral("\\int_0^\\infty"), 14.0);
    CHECK_FALSE(img.isNull());
}

TEST_CASE("MathRenderer: subscript and superscript", "[math_renderer]") {
    QImage img = MathRenderer::render(QStringLiteral("V_{m} (\\mathrm{mV})"), 12.0);
    CHECK_FALSE(img.isNull());
}

TEST_CASE("MathRenderer: complex expression path", "[math_renderer]") {
    QPainterPath path = MathRenderer::renderToPath(
        QStringLiteral("\\frac{\\partial f}{\\partial x}"), 12.0);
    CHECK_FALSE(path.isEmpty());
}
