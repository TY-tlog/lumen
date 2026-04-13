#include "FontEmbedder.h"

#include <QFontDatabase>
#include <QRawFont>

#include <QDebug>
#include <QHash>

namespace lumen::exp {

struct FontEmbedder::Impl {
    QStringList builtinNames;
    QHash<QString, QByteArray> registeredFonts;
};

FontEmbedder::FontEmbedder()
    : impl_(std::make_unique<Impl>())
{
    registerBuiltinFonts();
}

FontEmbedder::~FontEmbedder() = default;
FontEmbedder::FontEmbedder(FontEmbedder&&) noexcept = default;
FontEmbedder& FontEmbedder::operator=(FontEmbedder&&) noexcept = default;

bool FontEmbedder::registerFont(const QString& family, const QByteArray& fontData)
{
    if (family.isEmpty() || fontData.isEmpty())
        return false;
    impl_->registeredFonts[family] = fontData;
    return true;
}

std::optional<QByteArray> FontEmbedder::buildSubset(
    const QString& family, const QSet<QChar>& usedGlyphs) const
{
    // If we have raw font data, return it (Qt handles subset in PDF/SVG).
    auto it = impl_->registeredFonts.find(family);
    if (it != impl_->registeredFonts.end())
        return it.value();

    // For system fonts, try QRawFont to extract data.
    QFont font(family);
    font.setPixelSize(12);
    QRawFont rawFont = QRawFont::fromFont(font);
    if (!rawFont.isValid())
        return std::nullopt;

    QByteArray fontData = rawFont.fontTable("head");
    if (fontData.isEmpty()) {
        // Can't extract tables — return empty but valid optional
        // to indicate the font exists but subset not available.
        return QByteArray();
    }

    Q_UNUSED(usedGlyphs)
    return fontData;
}

QStringList FontEmbedder::builtinFonts() const
{
    return impl_->builtinNames;
}

QStringList FontEmbedder::registeredFamilies() const
{
    QStringList result = impl_->builtinNames;
    for (auto it = impl_->registeredFonts.begin(); it != impl_->registeredFonts.end(); ++it) {
        if (!result.contains(it.key()))
            result.append(it.key());
    }
    return result;
}

bool FontEmbedder::hasFamily(const QString& family) const
{
    if (impl_->registeredFonts.contains(family))
        return true;
    return impl_->builtinNames.contains(family);
}

void FontEmbedder::registerBuiltinFonts()
{
    // Academic fonts per ADR-050.
    static const QStringList kAcademicFonts = {
        QStringLiteral("Computer Modern"),
        QStringLiteral("Liberation Serif"),
        QStringLiteral("Liberation Sans"),
        QStringLiteral("Source Serif Pro"),
    };

    QStringList systemFamilies = QFontDatabase::families();
    impl_->builtinNames.clear();

    for (const auto& name : kAcademicFonts) {
        if (systemFamilies.contains(name)) {
            impl_->builtinNames.append(name);
        } else {
            // Register as available even if not installed — user sees
            // the name and can load the font file.
            impl_->builtinNames.append(name);
        }
    }
}

}  // namespace lumen::exp
