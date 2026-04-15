#include <catch2/catch_test_macros.hpp>

#include <style/json_io.h>

#include <QJsonObject>

using lumen::style::validateStyleJson;

TEST_CASE("Schema: valid v1.0 passes", "[schema]") {
    QJsonObject obj;
    obj[QStringLiteral("lumen_style_version")] = QStringLiteral("1.0");
    CHECK(validateStyleJson(obj).isEmpty());
}

TEST_CASE("Schema: v1.1 passes (patch version)", "[schema]") {
    QJsonObject obj;
    obj[QStringLiteral("lumen_style_version")] = QStringLiteral("1.1");
    CHECK(validateStyleJson(obj).isEmpty());
}

TEST_CASE("Schema: missing version fails", "[schema]") {
    QJsonObject obj;
    obj[QStringLiteral("name")] = QStringLiteral("test");
    CHECK_FALSE(validateStyleJson(obj).isEmpty());
}

TEST_CASE("Schema: v2.0 fails (major version)", "[schema]") {
    QJsonObject obj;
    obj[QStringLiteral("lumen_style_version")] = QStringLiteral("2.0");
    QString err = validateStyleJson(obj);
    CHECK_FALSE(err.isEmpty());
    CHECK(err.contains(QStringLiteral("2.0")));
}

TEST_CASE("Schema: empty version string fails", "[schema]") {
    QJsonObject obj;
    obj[QStringLiteral("lumen_style_version")] = QStringLiteral("");
    CHECK_FALSE(validateStyleJson(obj).isEmpty());
}

TEST_CASE("Schema: extra fields tolerated", "[schema]") {
    QJsonObject obj;
    obj[QStringLiteral("lumen_style_version")] = QStringLiteral("1.0");
    obj[QStringLiteral("unknown_field")] = QStringLiteral("ok");
    CHECK(validateStyleJson(obj).isEmpty());
}
