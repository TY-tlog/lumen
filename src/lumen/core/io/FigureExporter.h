#pragma once

// Stub header for FigureExporter — provides the Options struct and
// exportFigure signature so that ExportDialog and MainWindow compile
// before the real T5 implementation lands.  T5 will replace this file
// with the full implementation.

#include <QString>

namespace lumen::plot {
class PlotScene;
}

namespace lumen::core::io {

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

    /// Returns empty string on success, error message on failure.
    static QString exportFigure(const plot::PlotScene* scene,
                                const Options& opts);
};

}  // namespace lumen::core::io
