#pragma once

#include <QColor>
#include <QImage>
#include <QPainterPath>
#include <QString>

namespace lumen::exp {

/// Renders LaTeX math expressions to QImage or QPainterPath.
///
/// Phase 9 baseline: converts common LaTeX patterns to Unicode
/// equivalents and renders via Qt text engine. Full MicroTeX
/// integration is a future enhancement (ADR-052).
///
/// Supported patterns: Greek letters (\\alpha, \\beta, etc.),
/// superscripts (^{}), subscripts (_{}), \\sigma, \\mu, \\pi,
/// common operators, \\frac{}{} (rendered as a/b).
class MathRenderer {
public:
    /// Render LaTeX to a raster QImage.
    static QImage render(const QString& latex, double pointSize,
                         QColor color = Qt::black);

    /// Render LaTeX to a vector QPainterPath (for SVG/PDF export).
    static QPainterPath renderToPath(const QString& latex,
                                      double pointSize);

    /// Check if the LaTeX string is parseable (no crash guarantee).
    static bool isValidLatex(const QString& latex);

private:
    /// Convert LaTeX string to Unicode approximation.
    static QString latexToUnicode(const QString& latex);
};

}  // namespace lumen::exp
