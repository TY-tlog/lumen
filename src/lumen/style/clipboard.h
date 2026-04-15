#pragma once

#include "types.h"

#include <QString>

namespace lumen::style {

/// Style clipboard for copy/paste operations.
///
/// Stores a captured Style (resolved or override-only) that can
/// be pasted onto another element. Cross-element-type paste drops
/// incompatible properties.
class StyleClipboard {
public:
    /// Copy a style (resolved or override-only).
    void copy(const Style& style, const QString& sourceElement);

    /// Paste onto a target element. Returns the applicable subset.
    /// incompatibleCount is set to the number of dropped properties.
    Style paste(const QString& targetElement, int& incompatibleCount) const;

    /// Whether the clipboard has content.
    [[nodiscard]] bool hasContent() const { return hasContent_; }

    /// Source element name.
    [[nodiscard]] QString sourceElement() const { return sourceElement_; }

    /// Clear the clipboard.
    void clear();

private:
    Style style_;
    QString sourceElement_;
    bool hasContent_ = false;
};

}  // namespace lumen::style
