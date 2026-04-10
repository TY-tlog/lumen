#pragma once

#include <QString>

namespace lumen::plot {
class PlotScene;
}  // namespace lumen::plot

namespace lumen::core::io {

/// Exports a PlotScene to PNG, SVG, or PDF files via PlotRenderer.
///
/// Reuses the single PlotRenderer::render() code path so exported
/// figures look identical to on-screen display (ADR-026).
class FigureExporter {
public:
    enum class Format { Png, Svg, Pdf };

    struct Options {
        Format format = Format::Png;
        int widthPx = 1050;
        int heightPx = 700;
        int dpi = 300;
        bool transparentBackground = false;
        QString outputPath;
    };

    /// Renders the given PlotScene to a file.
    /// Returns empty string on success, error message on failure.
    static QString exportFigure(const plot::PlotScene* scene,
                                const Options& opts);

private:
    static QString exportPng(const plot::PlotScene* scene, const Options& opts);
    static QString exportSvg(const plot::PlotScene* scene, const Options& opts);
    static QString exportPdf(const plot::PlotScene* scene, const Options& opts);
};

}  // namespace lumen::core::io
