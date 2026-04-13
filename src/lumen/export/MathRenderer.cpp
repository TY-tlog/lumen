#include "MathRenderer.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QRegularExpression>

#include <QHash>

namespace lumen::exp {

namespace {

/// LaTeX command → Unicode character mapping.
const QHash<QString, QString>& greekMap()
{
    static const QHash<QString, QString> map = {
        {QStringLiteral("\\alpha"), QStringLiteral("\u03B1")},
        {QStringLiteral("\\beta"), QStringLiteral("\u03B2")},
        {QStringLiteral("\\gamma"), QStringLiteral("\u03B3")},
        {QStringLiteral("\\delta"), QStringLiteral("\u03B4")},
        {QStringLiteral("\\epsilon"), QStringLiteral("\u03B5")},
        {QStringLiteral("\\zeta"), QStringLiteral("\u03B6")},
        {QStringLiteral("\\eta"), QStringLiteral("\u03B7")},
        {QStringLiteral("\\theta"), QStringLiteral("\u03B8")},
        {QStringLiteral("\\iota"), QStringLiteral("\u03B9")},
        {QStringLiteral("\\kappa"), QStringLiteral("\u03BA")},
        {QStringLiteral("\\lambda"), QStringLiteral("\u03BB")},
        {QStringLiteral("\\mu"), QStringLiteral("\u03BC")},
        {QStringLiteral("\\nu"), QStringLiteral("\u03BD")},
        {QStringLiteral("\\xi"), QStringLiteral("\u03BE")},
        {QStringLiteral("\\pi"), QStringLiteral("\u03C0")},
        {QStringLiteral("\\rho"), QStringLiteral("\u03C1")},
        {QStringLiteral("\\sigma"), QStringLiteral("\u03C3")},
        {QStringLiteral("\\tau"), QStringLiteral("\u03C4")},
        {QStringLiteral("\\phi"), QStringLiteral("\u03C6")},
        {QStringLiteral("\\chi"), QStringLiteral("\u03C7")},
        {QStringLiteral("\\psi"), QStringLiteral("\u03C8")},
        {QStringLiteral("\\omega"), QStringLiteral("\u03C9")},
        {QStringLiteral("\\Gamma"), QStringLiteral("\u0393")},
        {QStringLiteral("\\Delta"), QStringLiteral("\u0394")},
        {QStringLiteral("\\Theta"), QStringLiteral("\u0398")},
        {QStringLiteral("\\Lambda"), QStringLiteral("\u039B")},
        {QStringLiteral("\\Sigma"), QStringLiteral("\u03A3")},
        {QStringLiteral("\\Phi"), QStringLiteral("\u03A6")},
        {QStringLiteral("\\Psi"), QStringLiteral("\u03A8")},
        {QStringLiteral("\\Omega"), QStringLiteral("\u03A9")},
        {QStringLiteral("\\infty"), QStringLiteral("\u221E")},
        {QStringLiteral("\\pm"), QStringLiteral("\u00B1")},
        {QStringLiteral("\\times"), QStringLiteral("\u00D7")},
        {QStringLiteral("\\cdot"), QStringLiteral("\u00B7")},
        {QStringLiteral("\\leq"), QStringLiteral("\u2264")},
        {QStringLiteral("\\geq"), QStringLiteral("\u2265")},
        {QStringLiteral("\\neq"), QStringLiteral("\u2260")},
        {QStringLiteral("\\approx"), QStringLiteral("\u2248")},
        {QStringLiteral("\\sqrt"), QStringLiteral("\u221A")},
        {QStringLiteral("\\int"), QStringLiteral("\u222B")},
        {QStringLiteral("\\sum"), QStringLiteral("\u2211")},
        {QStringLiteral("\\prod"), QStringLiteral("\u220F")},
        {QStringLiteral("\\partial"), QStringLiteral("\u2202")},
        {QStringLiteral("\\nabla"), QStringLiteral("\u2207")},
        {QStringLiteral("\\rightarrow"), QStringLiteral("\u2192")},
        {QStringLiteral("\\leftarrow"), QStringLiteral("\u2190")},
    };
    return map;
}

/// Unicode superscript digit map.
const QHash<QChar, QChar>& superscriptMap()
{
    static const QHash<QChar, QChar> map = {
        {u'0', u'\u2070'}, {u'1', u'\u00B9'}, {u'2', u'\u00B2'},
        {u'3', u'\u00B3'}, {u'4', u'\u2074'}, {u'5', u'\u2075'},
        {u'6', u'\u2076'}, {u'7', u'\u2077'}, {u'8', u'\u2078'},
        {u'9', u'\u2079'}, {u'n', u'\u207F'}, {u'i', u'\u2071'},
        {u'+', u'\u207A'}, {u'-', u'\u207B'}, {u'(', u'\u207D'},
        {u')', u'\u207E'},
    };
    return map;
}

/// Unicode subscript digit map.
const QHash<QChar, QChar>& subscriptMap()
{
    static const QHash<QChar, QChar> map = {
        {u'0', u'\u2080'}, {u'1', u'\u2081'}, {u'2', u'\u2082'},
        {u'3', u'\u2083'}, {u'4', u'\u2084'}, {u'5', u'\u2085'},
        {u'6', u'\u2086'}, {u'7', u'\u2087'}, {u'8', u'\u2088'},
        {u'9', u'\u2089'}, {u'+', u'\u208A'}, {u'-', u'\u208B'},
        {u'(', u'\u208D'}, {u')', u'\u208E'},
    };
    return map;
}

}  // namespace

QString MathRenderer::latexToUnicode(const QString& latex)
{
    QString result = latex;

    // Strip $ delimiters.
    if (result.startsWith(u'$'))
        result = result.mid(1);
    if (result.endsWith(u'$'))
        result.chop(1);

    // Replace \\mathrm{...} and \\text{...} with content.
    static const QRegularExpression rmRx(QStringLiteral("\\\\(?:mathrm|text)\\{([^}]*)\\}"));
    result.replace(rmRx, QStringLiteral("\\1"));

    // Replace \\frac{a}{b} with a/b.
    static const QRegularExpression fracRx(QStringLiteral("\\\\frac\\{([^}]*)\\}\\{([^}]*)\\}"));
    result.replace(fracRx, QStringLiteral("\\1/\\2"));

    // Replace Greek letters and symbols.
    for (auto it = greekMap().begin(); it != greekMap().end(); ++it) {
        result.replace(it.key(), it.value());
    }

    // Replace superscripts: ^{content} or ^c (single char).
    static const QRegularExpression supBraceRx(QStringLiteral("\\^\\{([^}]*)\\}"));
    auto supIt = supBraceRx.globalMatch(result);
    while (supIt.hasNext()) {
        auto match = supIt.next();
        QString content = match.captured(1);
        QString sup;
        for (const auto& ch : content) {
            auto mapped = superscriptMap().find(ch);
            sup += (mapped != superscriptMap().end()) ? mapped.value() : ch;
        }
        result.replace(match.captured(0), sup);
    }

    // Single-char superscript.
    static const QRegularExpression supCharRx(QStringLiteral("\\^(\\w)"));
    auto supCharIt = supCharRx.globalMatch(result);
    while (supCharIt.hasNext()) {
        auto match = supCharIt.next();
        QChar ch = match.captured(1).at(0);
        auto mapped = superscriptMap().find(ch);
        QString sup = (mapped != superscriptMap().end()) ? QString(mapped.value()) : match.captured(1);
        result.replace(match.captured(0), sup);
    }

    // Replace subscripts: _{content} or _c.
    static const QRegularExpression subBraceRx(QStringLiteral("_\\{([^}]*)\\}"));
    auto subIt = subBraceRx.globalMatch(result);
    while (subIt.hasNext()) {
        auto match = subIt.next();
        QString content = match.captured(1);
        QString sub;
        for (const auto& ch : content) {
            auto mapped = subscriptMap().find(ch);
            sub += (mapped != subscriptMap().end()) ? mapped.value() : ch;
        }
        result.replace(match.captured(0), sub);
    }

    static const QRegularExpression subCharRx(QStringLiteral("_(\\w)"));
    auto subCharIt = subCharRx.globalMatch(result);
    while (subCharIt.hasNext()) {
        auto match = subCharIt.next();
        QChar ch = match.captured(1).at(0);
        auto mapped = subscriptMap().find(ch);
        QString sub = (mapped != subscriptMap().end()) ? QString(mapped.value()) : match.captured(1);
        result.replace(match.captured(0), sub);
    }

    // Remove remaining braces.
    result.remove(u'{');
    result.remove(u'}');

    return result;
}

QImage MathRenderer::render(const QString& latex, double pointSize, QColor color)
{
    QString text = latexToUnicode(latex);

    QFont font;
    font.setPointSizeF(pointSize);
    QFontMetricsF fm(font);
    QRectF bounds = fm.boundingRect(text);
    if (bounds.isEmpty())
        bounds = QRectF(0, 0, 10, static_cast<int>(pointSize + 4));

    int w = static_cast<int>(bounds.width() + 4);
    int h = static_cast<int>(bounds.height() + 4);
    QImage image(w, h, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setFont(font);
    painter.setPen(color);
    painter.drawText(QRectF(0, 0, w, h), Qt::AlignCenter, text);
    painter.end();

    return image;
}

QPainterPath MathRenderer::renderToPath(const QString& latex, double pointSize)
{
    QString text = latexToUnicode(latex);

    QFont font;
    font.setPointSizeF(pointSize);

    QPainterPath path;
    path.addText(0, QFontMetricsF(font).ascent(), font, text);
    return path;
}

bool MathRenderer::isValidLatex(const QString& latex)
{
    if (latex.isEmpty())
        return false;

    // Check for unmatched braces.
    int depth = 0;
    for (const auto& ch : latex) {
        if (ch == u'{') ++depth;
        if (ch == u'}') --depth;
        if (depth < 0) return false;
    }
    if (depth != 0)
        return false;

    // Check for unknown commands (single backslash followed by nothing).
    if (latex.endsWith(u'\\'))
        return false;

    return true;
}

}  // namespace lumen::exp
