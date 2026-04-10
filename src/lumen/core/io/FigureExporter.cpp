// Stub implementation for FigureExporter.
// T5 will replace this with the real PNG/SVG/PDF export logic.

#include "FigureExporter.h"

namespace lumen::core::io {

QString FigureExporter::exportFigure(const plot::PlotScene* /*scene*/,
                                     const Options& /*opts*/) {
    return QStringLiteral("FigureExporter not yet implemented (awaiting T5)");
}

}  // namespace lumen::core::io
