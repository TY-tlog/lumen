#include <catch2/catch_test_macros.hpp>

#include <export/ColorProfile.h>

#include <cmath>

using lumen::exp::ColorProfile;

TEST_CASE("ColorProfile: sRGB to sRGB conversion is identity", "[color_conversion]") {
    auto srgb = ColorProfile::builtin(ColorProfile::Builtin::sRGB);
    QColor input(128, 64, 200);
    QColor output = srgb.convert(input, srgb);
    // Should be close to identity (minor rounding from 8-bit quantization).
    CHECK(std::abs(output.red() - input.red()) <= 2);
    CHECK(std::abs(output.green() - input.green()) <= 2);
    CHECK(std::abs(output.blue() - input.blue()) <= 2);
}

TEST_CASE("ColorProfile: sRGB to Gray produces grayscale", "[color_conversion]") {
    auto srgb = ColorProfile::builtin(ColorProfile::Builtin::sRGB);
    auto gray = ColorProfile::builtin(ColorProfile::Builtin::Gray_Gamma22);
    QColor input(128, 64, 200);
    QColor output = srgb.convert(input, gray);
    // Grayscale: R==G==B.
    CHECK(output.red() == output.green());
    CHECK(output.green() == output.blue());
}

TEST_CASE("ColorProfile: sRGB pure white converts to near-white in AdobeRGB", "[color_conversion]") {
    auto srgb = ColorProfile::builtin(ColorProfile::Builtin::sRGB);
    auto adobe = ColorProfile::builtin(ColorProfile::Builtin::AdobeRGB);
    QColor white(255, 255, 255);
    QColor output = srgb.convert(white, adobe);
    // White should map to near-white (within gamut).
    CHECK(output.red() >= 250);
    CHECK(output.green() >= 250);
    CHECK(output.blue() >= 250);
}

TEST_CASE("ColorProfile: sRGB pure black converts to black", "[color_conversion]") {
    auto srgb = ColorProfile::builtin(ColorProfile::Builtin::sRGB);
    auto adobe = ColorProfile::builtin(ColorProfile::Builtin::AdobeRGB);
    QColor black(0, 0, 0);
    QColor output = srgb.convert(black, adobe);
    CHECK(output.red() <= 2);
    CHECK(output.green() <= 2);
    CHECK(output.blue() <= 2);
}

TEST_CASE("ColorProfile: invalid profile convert returns input unchanged", "[color_conversion]") {
    // Create an invalid profile by loading garbage data.
    auto invalid = ColorProfile::fromIccData(QByteArray("not valid"));
    auto srgb = ColorProfile::builtin(ColorProfile::Builtin::sRGB);
    QColor input(100, 150, 200);
    QColor output = invalid.convert(input, srgb);
    CHECK(output == input);
}
