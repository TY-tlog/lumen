#include <catch2/catch_test_macros.hpp>

#include <export/ColorProfile.h>

#include <QDir>
#include <QFile>

using lumen::exp::ColorProfile;

TEST_CASE("ColorProfile: fromIccData round-trips sRGB profile", "[color_profile]") {
    auto original = ColorProfile::builtin(ColorProfile::Builtin::sRGB);
    QByteArray bytes = original.iccBytes();
    REQUIRE_FALSE(bytes.isEmpty());

    auto loaded = ColorProfile::fromIccData(bytes);
    CHECK(loaded.isValid());
    CHECK(loaded.isRGB());
    CHECK_FALSE(loaded.iccBytes().isEmpty());
}

TEST_CASE("ColorProfile: fromIccData with invalid data returns invalid", "[color_profile]") {
    QByteArray garbage("not an icc profile");
    auto loaded = ColorProfile::fromIccData(garbage);
    CHECK_FALSE(loaded.isValid());
}

TEST_CASE("ColorProfile: fromIccFile with nonexistent path returns invalid", "[color_profile]") {
    auto loaded = ColorProfile::fromIccFile(QStringLiteral("/tmp/nonexistent_profile.icc"));
    CHECK_FALSE(loaded.isValid());
}

TEST_CASE("ColorProfile: fromIccFile round-trips via temp file", "[color_profile]") {
    auto original = ColorProfile::builtin(ColorProfile::Builtin::AdobeRGB);
    QByteArray bytes = original.iccBytes();

    QString path = QDir::tempPath() + QStringLiteral("/lumen_test_profile.icc");
    QFile f(path);
    REQUIRE(f.open(QIODevice::WriteOnly));
    f.write(bytes);
    f.close();

    auto loaded = ColorProfile::fromIccFile(path);
    CHECK(loaded.isValid());
    CHECK(loaded.isRGB());

    QFile::remove(path);
}
