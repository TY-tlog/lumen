#pragma once

#include <QByteArray>
#include <QFont>
#include <QSet>
#include <QString>
#include <QStringList>

#include <memory>
#include <optional>

namespace lumen::exp {

/// Font registration and subset building for publication-grade export.
///
/// Probes system QFontDatabase for academic fonts at construction.
/// Additional fonts registered via registerFont(). Qt's built-in
/// PDF/SVG writers handle actual font embedding (ADR-050).
class FontEmbedder {
public:
    FontEmbedder();
    ~FontEmbedder();

    FontEmbedder(const FontEmbedder&) = delete;
    FontEmbedder& operator=(const FontEmbedder&) = delete;
    FontEmbedder(FontEmbedder&&) noexcept;
    FontEmbedder& operator=(FontEmbedder&&) noexcept;

    bool registerFont(const QString& family, const QByteArray& fontData);

    [[nodiscard]] std::optional<QByteArray> buildSubset(
        const QString& family, const QSet<QChar>& usedGlyphs) const;

    [[nodiscard]] QStringList builtinFonts() const;
    [[nodiscard]] QStringList registeredFamilies() const;
    [[nodiscard]] bool hasFamily(const QString& family) const;

    void registerBuiltinFonts();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace lumen::exp
