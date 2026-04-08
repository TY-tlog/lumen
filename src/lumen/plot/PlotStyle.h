#pragma once

#include "style/DesignTokens.h"

#include <QColor>
#include <Qt>

namespace lumen::plot {

/// Visual style properties for a single plot series.
struct PlotStyle {
    QColor color;
    double lineWidth{tokens::plot::defaultLineWidth};
    Qt::PenStyle penStyle{Qt::SolidLine};

    /// Create a PlotStyle using the palette color at the given index.
    [[nodiscard]] static PlotStyle fromPalette(int index)
    {
        const auto& palette = tokens::color::plotPalette;
        const int idx = index % static_cast<int>(palette.size());
        return PlotStyle{palette[static_cast<size_t>(idx)],
                         tokens::plot::defaultLineWidth, Qt::SolidLine};
    }
};

} // namespace lumen::plot
