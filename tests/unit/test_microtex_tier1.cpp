#include <catch2/catch_test_macros.hpp>

#include <export/MathRenderer.h>

using lumen::exp::MathRenderer;

// Tier 1 macros: each must render to a non-empty image and path.

TEST_CASE("Tier1: Greek letters render", "[microtex][tier1]") {
    auto img = MathRenderer::render(QStringLiteral("\\alpha + \\beta = \\gamma"), 12.0);
    CHECK_FALSE(img.isNull());
    CHECK(img.width() > 0);
}

TEST_CASE("Tier1: superscript renders", "[microtex][tier1]") {
    auto img = MathRenderer::render(QStringLiteral("\\sigma^2"), 12.0);
    CHECK_FALSE(img.isNull());
}

TEST_CASE("Tier1: subscript renders", "[microtex][tier1]") {
    auto img = MathRenderer::render(QStringLiteral("x_{i}"), 12.0);
    CHECK_FALSE(img.isNull());
}

TEST_CASE("Tier1: frac renders", "[microtex][tier1]") {
    auto img = MathRenderer::render(QStringLiteral("\\frac{a}{b}"), 12.0);
    CHECK_FALSE(img.isNull());
    auto path = MathRenderer::renderToPath(QStringLiteral("\\frac{a}{b}"), 12.0);
    CHECK_FALSE(path.isEmpty());
}

TEST_CASE("Tier1: sqrt renders", "[microtex][tier1]") {
    auto img = MathRenderer::render(QStringLiteral("\\sqrt{x^2 + y^2}"), 12.0);
    CHECK_FALSE(img.isNull());
}

TEST_CASE("Tier1: sqrt with index renders", "[microtex][tier1]") {
    auto img = MathRenderer::render(QStringLiteral("\\sqrt[3]{27}"), 12.0);
    CHECK_FALSE(img.isNull());
}

TEST_CASE("Tier1: sum with limits", "[microtex][tier1]") {
    auto img = MathRenderer::render(QStringLiteral("\\sum_{i=0}^{n} x_i"), 12.0);
    CHECK_FALSE(img.isNull());
}

TEST_CASE("Tier1: integral with limits", "[microtex][tier1]") {
    auto img = MathRenderer::render(QStringLiteral("\\int_0^\\infty e^{-x} dx"), 12.0);
    CHECK_FALSE(img.isNull());
}

TEST_CASE("Tier1: operators pm mp times cdot div", "[microtex][tier1]") {
    CHECK_FALSE(MathRenderer::render(QStringLiteral("a \\pm b"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("a \\mp b"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("a \\times b"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("a \\cdot b"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("a \\div b"), 12.0).isNull());
}

TEST_CASE("Tier1: relations leq geq neq approx equiv sim propto", "[microtex][tier1]") {
    CHECK_FALSE(MathRenderer::render(QStringLiteral("a \\leq b"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("a \\geq b"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("a \\neq b"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("a \\approx b"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("a \\equiv b"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("a \\sim b"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("a \\propto b"), 12.0).isNull());
}

TEST_CASE("Tier1: mathrm and mathbf", "[microtex][tier1]") {
    CHECK_FALSE(MathRenderer::render(QStringLiteral("\\mathrm{Re}(z)"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("\\mathbf{E}"), 12.0).isNull());
}

TEST_CASE("Tier1: accents hat bar vec tilde dot ddot", "[microtex][tier1]") {
    CHECK_FALSE(MathRenderer::render(QStringLiteral("\\hat{x}"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("\\bar{x}"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("\\vec{F}"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("\\tilde{x}"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("\\dot{x}"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("\\ddot{q}"), 12.0).isNull());
}

TEST_CASE("Tier1: delimiters left right", "[microtex][tier1]") {
    auto img = MathRenderer::render(QStringLiteral("\\left( \\frac{a}{b} \\right)"), 12.0);
    CHECK_FALSE(img.isNull());
}

TEST_CASE("Tier1: partial and nabla", "[microtex][tier1]") {
    CHECK_FALSE(MathRenderer::render(QStringLiteral("\\partial f"), 12.0).isNull());
    CHECK_FALSE(MathRenderer::render(QStringLiteral("\\nabla \\cdot \\mathbf{E}"), 12.0).isNull());
}

TEST_CASE("Tier1: all 20 corpus equations render", "[microtex][tier1]") {
    // Comprehensive: every Tier 1 corpus equation must produce non-null image.
    QStringList tier1 = {
        QStringLiteral("\\alpha + \\beta = \\gamma"),
        QStringLiteral("\\sigma^2"),
        QStringLiteral("x_{i} + y_{j}"),
        QStringLiteral("\\frac{a}{b}"),
        QStringLiteral("\\sqrt{x^2 + y^2}"),
        QStringLiteral("\\sqrt[3]{27}"),
        QStringLiteral("\\sum_{i=0}^{n} x_i"),
        QStringLiteral("\\int_0^\\infty e^{-x} dx"),
        QStringLiteral("\\prod_{k=1}^{N} a_k"),
        QStringLiteral("\\partial f / \\partial x"),
        QStringLiteral("\\nabla \\cdot \\mathbf{E} = \\frac{\\rho}{\\epsilon_0}"),
        QStringLiteral("E = mc^2"),
        QStringLiteral("\\Delta G = \\Delta H - T\\Delta S"),
        QStringLiteral("\\pm 1.96 \\sigma"),
        QStringLiteral("a \\leq b \\leq c"),
        QStringLiteral("x \\neq y, \\quad x \\approx z"),
        QStringLiteral("\\mathrm{Re}(z) + i\\,\\mathrm{Im}(z)"),
        QStringLiteral("\\hat{x} \\cdot \\hat{y} = 0"),
        QStringLiteral("\\vec{F} = m\\vec{a}"),
        QStringLiteral("\\left( \\frac{a}{b} \\right)"),
    };
    int passed = 0;
    for (const auto& eq : tier1) {
        auto img = MathRenderer::render(eq, 12.0);
        if (!img.isNull()) ++passed;
    }
    CHECK(passed == 20);
}
