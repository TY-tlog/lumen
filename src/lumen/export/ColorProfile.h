#pragma once

#include <QByteArray>
#include <QColor>
#include <QString>

#include <memory>

// Forward-declare lcms2 handle type to avoid leaking lcms2.h into callers.
using cmsHPROFILE = void*;

namespace lumen::exp {

/// ICC color profile wrapper backed by LittleCMS (lcms2).
///
/// Provides built-in profiles for common screen and print color spaces,
/// conversion between profiles, and ICC binary data for embedding in
/// PNG (iCCP chunk) and PDF (/ICCBased ColorSpace).
class ColorProfile {
public:
    enum class Builtin {
        sRGB,                   ///< IEC 61966-2.1, screen default
        AdobeRGB,               ///< Adobe RGB (1998), wide-gamut
        DisplayP3,              ///< Apple Display P3
        CMYK_USWebCoatedSWOP,   ///< US Web Coated (SWOP) v2
        CMYK_FOGRA39,           ///< European print standard
        Gray_Gamma22            ///< Grayscale gamma 2.2
    };

    /// Create a built-in profile.
    static ColorProfile builtin(Builtin id);

    /// Load an ICC profile from a file on disk.
    /// Returns an invalid profile if the file cannot be read or parsed.
    static ColorProfile fromIccFile(const QString& path);

    /// Load an ICC profile from raw ICC data bytes.
    static ColorProfile fromIccData(const QByteArray& data);

    ~ColorProfile();
    ColorProfile(const ColorProfile& other);
    ColorProfile& operator=(const ColorProfile& other);
    ColorProfile(ColorProfile&& other) noexcept;
    ColorProfile& operator=(ColorProfile&& other) noexcept;

    /// Whether this profile was created/loaded successfully.
    [[nodiscard]] bool isValid() const;

    /// Raw ICC profile bytes (for embedding in PNG/PDF).
    [[nodiscard]] QByteArray iccBytes() const;

    /// Human-readable profile name.
    [[nodiscard]] QString name() const;

    /// Color space queries.
    [[nodiscard]] bool isCMYK() const;
    [[nodiscard]] bool isRGB() const;
    [[nodiscard]] bool isGrayscale() const;

    /// Convert a color from this profile's space to the target profile.
    [[nodiscard]] QColor convert(QColor source, const ColorProfile& target) const;

    /// Access the underlying lcms2 handle (for advanced usage).
    [[nodiscard]] cmsHPROFILE handle() const;

private:
    ColorProfile() = default;
    explicit ColorProfile(cmsHPROFILE profile, QString profileName);

    struct Impl;
    std::shared_ptr<Impl> impl_;
};

}  // namespace lumen::exp
