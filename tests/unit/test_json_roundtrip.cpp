#include <catch2/catch_test_macros.hpp>

#include <style/json_io.h>
#include <style/types.h>

using namespace lumen::style;

TEST_CASE("JSON roundtrip: empty style", "[json_roundtrip]") {
    Style original;
    QJsonObject json = saveStyleToJson(original);
    Style loaded = loadStyleFromJson(json);
    CHECK(loaded == original);
}

TEST_CASE("JSON roundtrip: backgroundColor", "[json_roundtrip]") {
    Style original;
    original.backgroundColor = QColor(255, 255, 255);
    QJsonObject json = saveStyleToJson(original, "test");
    Style loaded = loadStyleFromJson(json);
    REQUIRE(loaded.backgroundColor.has_value());
    CHECK(*loaded.backgroundColor == QColor(255, 255, 255));
}

TEST_CASE("JSON roundtrip: stroke sub-style", "[json_roundtrip]") {
    Style original;
    original.stroke = StrokeStyle{QColor(Qt::red), 2.0, std::nullopt, std::nullopt, std::nullopt};
    QJsonObject json = saveStyleToJson(original);
    Style loaded = loadStyleFromJson(json);
    REQUIRE(loaded.stroke.has_value());
    REQUIRE(loaded.stroke->color.has_value());
    CHECK(loaded.stroke->color->red() == 255);
    CHECK(*loaded.stroke->width == 2.0);
}

TEST_CASE("JSON roundtrip: text family and size", "[json_roundtrip]") {
    Style original;
    original.text = TextStyle{QString("Inter"), 12.0, std::nullopt, std::nullopt, std::nullopt};
    QJsonObject json = saveStyleToJson(original);
    Style loaded = loadStyleFromJson(json);
    REQUIRE(loaded.text.has_value());
    CHECK(*loaded.text->family == QString("Inter"));
    CHECK(*loaded.text->size == 12.0);
}

TEST_CASE("JSON roundtrip: name and extends preserved", "[json_roundtrip]") {
    Style original;
    original.lineWidth = 1.5;
    QJsonObject json = saveStyleToJson(original, "my-theme", "publication");
    CHECK(json[QStringLiteral("name")].toString() == QStringLiteral("my-theme"));
    CHECK(json[QStringLiteral("extends")].toString() == QStringLiteral("publication"));
    CHECK(json[QStringLiteral("lumen_style_version")].toString() == QStringLiteral("1.0"));
}

TEST_CASE("JSON roundtrip: nullopt properties omitted", "[json_roundtrip]") {
    Style original;
    original.lineWidth = 1.5;
    // markerSize not set
    QJsonObject json = saveStyleToJson(original);
    CHECK(json.contains(QStringLiteral("lineWidth")));
    CHECK_FALSE(json.contains(QStringLiteral("markerSize")));
}
