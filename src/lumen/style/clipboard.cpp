#include "clipboard.h"

namespace lumen::style {

void StyleClipboard::copy(const Style& style, const QString& sourceElement)
{
    style_ = style;
    sourceElement_ = sourceElement;
    hasContent_ = true;
}

Style StyleClipboard::paste(const QString& targetElement, int& incompatibleCount) const
{
    incompatibleCount = 0;

    if (!hasContent_)
        return {};

    // For same element type, return full style.
    // For different types, drop type-specific properties.
    // Element types are distinguished by prefix (series.line vs axis.spine).
    QString sourcePrefix = sourceElement_.section(u'.', 0, 0);
    QString targetPrefix = targetElement.section(u'.', 0, 0);

    if (sourcePrefix == targetPrefix) {
        // Same category — full copy.
        return style_;
    }

    // Cross-type paste: keep only universally applicable properties.
    Style result;
    int total = 0;
    int kept = 0;

    // Stroke is universal.
    if (style_.stroke.has_value()) { result.stroke = style_.stroke; ++kept; ++total; }
    // Fill is universal.
    if (style_.fill.has_value()) { result.fill = style_.fill; ++kept; ++total; }
    // Text is universal.
    if (style_.text.has_value()) { result.text = style_.text; ++kept; ++total; }

    // Marker is series-specific.
    if (style_.marker.has_value()) {
        ++total;
        if (targetPrefix == QStringLiteral("series")) {
            result.marker = style_.marker;
            ++kept;
        }
    }

    // Grid is grid-specific.
    if (style_.grid.has_value()) {
        ++total;
        if (targetPrefix == QStringLiteral("grid")) {
            result.grid = style_.grid;
            ++kept;
        }
    }

    // Top-level scalars.
    if (style_.backgroundColor.has_value()) { result.backgroundColor = style_.backgroundColor; ++kept; ++total; }
    if (style_.foregroundColor.has_value()) { result.foregroundColor = style_.foregroundColor; ++kept; ++total; }
    if (style_.lineWidth.has_value()) { result.lineWidth = style_.lineWidth; ++kept; ++total; }

    incompatibleCount = total - kept;
    return result;
}

void StyleClipboard::clear()
{
    style_ = {};
    sourceElement_.clear();
    hasContent_ = false;
}

}  // namespace lumen::style
