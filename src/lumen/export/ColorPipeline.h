#pragma once

#include "ColorProfile.h"

#include <QByteArray>
#include <QImage>
#include <QString>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::exp {

/// Color pipeline that wraps FigureExporter to apply ICC color
/// management during export.
///
/// For PNG: sets the QImage color space from the ICC profile,
///   which causes Qt to embed the iCCP chunk automatically.
/// For PDF: embeds the ICC profile bytes as metadata (QPdfWriter
///   doesn't support /ICCBased directly, so we store the profile
///   name and provide iccBytes() for external tools).
/// For SVG: SVG assumes sRGB; no conversion needed.
class ColorPipeline {
public:
    /// Apply an ICC profile to a QImage before saving as PNG.
    /// Sets the color space on the image so Qt embeds iCCP.
    static void applyProfileToImage(QImage& image, const ColorProfile& profile);

    /// Get the ICC profile bytes for PDF embedding.
    /// The caller writes these into the PDF stream.
    [[nodiscard]] static QByteArray profileBytesForPdf(const ColorProfile& profile);

    /// Check if a color profile requires color conversion
    /// (i.e., it's not sRGB, which is the default working space).
    [[nodiscard]] static bool needsConversion(const ColorProfile& profile);
};

}  // namespace lumen::exp
