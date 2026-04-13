#include <catch2/catch_test_macros.hpp>

#include <export/MathRenderer.h>

using lumen::exp::MathRenderer;

TEST_CASE("MathRenderer: sigma^2 renders non-empty image", "[math_renderer]") {
    QImage img = MathRenderer::render(QStringLiteral("\\sigma^2"), 12.0);
    CHECK_FALSE(img.isNull());
    CHECK(img.width() > 0);
    CHECK(img.height() > 0);
}

TEST_CASE("MathRenderer: Greek letters render", "[math_renderer]") {
    QImage img = MathRenderer::render(QStringLiteral("\\alpha + \\beta"), 12.0);
    CHECK_FALSE(img.isNull());
}

TEST_CASE("MathRenderer: renderToPath returns non-empty path", "[math_renderer]") {
    QPainterPath path = MathRenderer::renderToPath(QStringLiteral("\\sigma^2"), 12.0);
    CHECK_FALSE(path.isEmpty());
}

TEST_CASE("MathRenderer: isValidLatex accepts valid expressions", "[math_renderer]") {
    CHECK(MathRenderer::isValidLatex(QStringLiteral("\\sigma^{2}")));
    CHECK(MathRenderer::isValidLatex(QStringLiteral("x + y")));
    CHECK(MathRenderer::isValidLatex(QStringLiteral("\\frac{a}{b}")));
}
