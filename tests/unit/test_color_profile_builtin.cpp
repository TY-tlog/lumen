#include <catch2/catch_test_macros.hpp>

#include <export/ColorProfile.h>

using lumen::exp::ColorProfile;

TEST_CASE("ColorProfile: sRGB builtin loads and is valid", "[color_profile]") {
    auto p = ColorProfile::builtin(ColorProfile::Builtin::sRGB);
    CHECK(p.isValid());
    CHECK(p.isRGB());
    CHECK_FALSE(p.isCMYK());
    CHECK_FALSE(p.isGrayscale());
    CHECK_FALSE(p.name().isEmpty());
}

TEST_CASE("ColorProfile: AdobeRGB builtin loads and is valid", "[color_profile]") {
    auto p = ColorProfile::builtin(ColorProfile::Builtin::AdobeRGB);
    CHECK(p.isValid());
    CHECK(p.isRGB());
}

TEST_CASE("ColorProfile: DisplayP3 builtin loads and is valid", "[color_profile]") {
    auto p = ColorProfile::builtin(ColorProfile::Builtin::DisplayP3);
    CHECK(p.isValid());
    CHECK(p.isRGB());
}

TEST_CASE("ColorProfile: Gray_Gamma22 builtin loads and is valid", "[color_profile]") {
    auto p = ColorProfile::builtin(ColorProfile::Builtin::Gray_Gamma22);
    CHECK(p.isValid());
    CHECK(p.isGrayscale());
    CHECK_FALSE(p.isRGB());
    CHECK_FALSE(p.isCMYK());
}

TEST_CASE("ColorProfile: all builtins produce non-empty ICC bytes", "[color_profile]") {
    auto srgb = ColorProfile::builtin(ColorProfile::Builtin::sRGB);
    auto adobe = ColorProfile::builtin(ColorProfile::Builtin::AdobeRGB);
    auto p3 = ColorProfile::builtin(ColorProfile::Builtin::DisplayP3);
    auto gray = ColorProfile::builtin(ColorProfile::Builtin::Gray_Gamma22);

    CHECK_FALSE(srgb.iccBytes().isEmpty());
    CHECK_FALSE(adobe.iccBytes().isEmpty());
    CHECK_FALSE(p3.iccBytes().isEmpty());
    CHECK_FALSE(gray.iccBytes().isEmpty());
}

TEST_CASE("ColorProfile: ICC bytes start with valid header", "[color_profile]") {
    auto p = ColorProfile::builtin(ColorProfile::Builtin::sRGB);
    QByteArray bytes = p.iccBytes();
    REQUIRE(bytes.size() > 128);
    // ICC profile magic: 'acsp' at offset 36.
    CHECK(bytes.at(36) == 'a');
    CHECK(bytes.at(37) == 'c');
    CHECK(bytes.at(38) == 's');
    CHECK(bytes.at(39) == 'p');
}
