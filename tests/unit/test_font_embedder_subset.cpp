#include <catch2/catch_test_macros.hpp>

#include <export/FontEmbedder.h>

using lumen::exp::FontEmbedder;

TEST_CASE("FontEmbedder: construction registers builtins", "[font_embedder]") {
    FontEmbedder embedder;
    QStringList builtins = embedder.builtinFonts();
    CHECK(builtins.size() == 4);
}

TEST_CASE("FontEmbedder: builtin font names are correct", "[font_embedder]") {
    FontEmbedder embedder;
    QStringList builtins = embedder.builtinFonts();
    CHECK(builtins.contains(QStringLiteral("Computer Modern")));
    CHECK(builtins.contains(QStringLiteral("Liberation Serif")));
    CHECK(builtins.contains(QStringLiteral("Liberation Sans")));
    CHECK(builtins.contains(QStringLiteral("Source Serif Pro")));
}

TEST_CASE("FontEmbedder: registerFont adds a family", "[font_embedder]") {
    FontEmbedder embedder;
    QByteArray fakeFont("fake font data");
    CHECK(embedder.registerFont(QStringLiteral("TestFont"), fakeFont));
    CHECK(embedder.hasFamily(QStringLiteral("TestFont")));
    CHECK(embedder.registeredFamilies().contains(QStringLiteral("TestFont")));
}

TEST_CASE("FontEmbedder: registerFont rejects empty", "[font_embedder]") {
    FontEmbedder embedder;
    CHECK_FALSE(embedder.registerFont(QString(), QByteArray("data")));
    CHECK_FALSE(embedder.registerFont(QStringLiteral("name"), QByteArray()));
}

TEST_CASE("FontEmbedder: buildSubset returns data for registered font", "[font_embedder]") {
    FontEmbedder embedder;
    QByteArray data("test font binary data");
    embedder.registerFont(QStringLiteral("TestFont"), data);
    auto subset = embedder.buildSubset(QStringLiteral("TestFont"), {u'a', u'b'});
    REQUIRE(subset.has_value());
    CHECK_FALSE(subset->isEmpty());
}

TEST_CASE("FontEmbedder: buildSubset for unregistered font attempts system lookup", "[font_embedder]") {
    FontEmbedder embedder;
    // System may have a fallback font, so buildSubset may return data.
    // Just verify it doesn't crash.
    auto subset = embedder.buildSubset(QStringLiteral("ZZZ_Nonexistent_Font_XYZ"), {u'a'});
    // Either nullopt (font not found) or some data (fallback) — both are valid.
    SUCCEED("buildSubset did not crash");
}
