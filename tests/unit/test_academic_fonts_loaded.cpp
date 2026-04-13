#include <catch2/catch_test_macros.hpp>

#include <export/FontEmbedder.h>

using lumen::exp::FontEmbedder;

TEST_CASE("Academic fonts: 4 built-in fonts registered", "[academic_fonts]") {
    FontEmbedder embedder;
    CHECK(embedder.builtinFonts().size() == 4);
}

TEST_CASE("Academic fonts: hasFamily for each builtin", "[academic_fonts]") {
    FontEmbedder embedder;
    CHECK(embedder.hasFamily(QStringLiteral("Computer Modern")));
    CHECK(embedder.hasFamily(QStringLiteral("Liberation Serif")));
    CHECK(embedder.hasFamily(QStringLiteral("Liberation Sans")));
    CHECK(embedder.hasFamily(QStringLiteral("Source Serif Pro")));
}

TEST_CASE("Academic fonts: registeredFamilies includes builtins", "[academic_fonts]") {
    FontEmbedder embedder;
    QStringList all = embedder.registeredFamilies();
    CHECK(all.size() >= 4);
}
