#include <catch2/catch_test_macros.hpp>

#include <export/MathRenderer.h>

using lumen::exp::MathRenderer;

TEST_CASE("MathRenderer: empty string is not valid", "[math_renderer]") {
    CHECK_FALSE(MathRenderer::isValidLatex(QString()));
    CHECK_FALSE(MathRenderer::isValidLatex(QStringLiteral("")));
}

TEST_CASE("MathRenderer: unmatched braces are invalid", "[math_renderer]") {
    CHECK_FALSE(MathRenderer::isValidLatex(QStringLiteral("{{")));
    CHECK_FALSE(MathRenderer::isValidLatex(QStringLiteral("}")));
    CHECK_FALSE(MathRenderer::isValidLatex(QStringLiteral("{{{"))); // 3 opens, 0 closes
}

TEST_CASE("MathRenderer: trailing backslash is invalid", "[math_renderer]") {
    CHECK_FALSE(MathRenderer::isValidLatex(QStringLiteral("x\\")));
}

TEST_CASE("MathRenderer: render of invalid LaTeX does not crash", "[math_renderer]") {
    // Even invalid input should not crash — just produce empty/garbage.
    QImage img = MathRenderer::render(QStringLiteral("{{{{"), 12.0);
    // No assertion on content — just verify no crash.
    (void)img;
}

TEST_CASE("MathRenderer: render of empty string returns image", "[math_renderer]") {
    QImage img = MathRenderer::render(QStringLiteral(""), 12.0);
    // Should return a small default image, not crash.
    CHECK_FALSE(img.isNull());
}
