#pragma once

#include "AnnotationLayer.h"
#include "Axis.h"
#include "Legend.h"
#include "LineSeries.h"
#include "PlotItem.h"
#include "ViewTransform.h"

#include <QFont>
#include <QFontMetrics>
#include <QRectF>
#include <QSizeF>
#include <QString>

#include <cassert>
#include <memory>
#include <vector>

namespace lumen::plot {

/// Margin values for the four sides of the plot area.
struct PlotMargins {
    double left = 0.0;
    double top = 0.0;
    double right = 0.0;
    double bottom = 0.0;
};

/// Lightweight view over a vector of PlotItem unique_ptrs that provides
/// const LineSeries& access for backward compatibility.  Supports
/// operator[], size(), and range-for iteration.
class LineSeriesView {
public:
    explicit LineSeriesView(
        const std::vector<std::unique_ptr<PlotItem>>& items)
        : items_(items) {}

    [[nodiscard]] const LineSeries& operator[](std::size_t i) const {
        const auto* ls = dynamic_cast<const LineSeries*>(items_[i].get());
        assert(ls && "LineSeriesView: item is not a LineSeries");
        return *ls;
    }
    [[nodiscard]] std::size_t size() const { return items_.size(); }
    [[nodiscard]] bool empty() const { return items_.empty(); }

    // Iterator support for range-for.
    class Iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = const LineSeries&;
        using pointer = const LineSeries*;
        using reference = const LineSeries&;
        using iterator_category = std::input_iterator_tag;

        Iterator(const std::vector<std::unique_ptr<PlotItem>>& items,
                 std::size_t pos)
            : items_(items), pos_(pos) {}
        const LineSeries& operator*() const {
            const auto* ls = dynamic_cast<const LineSeries*>(items_[pos_].get());
            assert(ls && "LineSeriesView::Iterator: item is not a LineSeries");
            return *ls;
        }
        Iterator& operator++() { ++pos_; return *this; }
        Iterator operator++(int) { auto tmp = *this; ++pos_; return tmp; }
        bool operator==(const Iterator& o) const { return pos_ == o.pos_; }
        bool operator!=(const Iterator& o) const { return pos_ != o.pos_; }
    private:
        const std::vector<std::unique_ptr<PlotItem>>& items_;
        std::size_t pos_;
    };

    [[nodiscard]] Iterator begin() const { return {items_, 0}; }
    [[nodiscard]] Iterator end() const { return {items_, items_.size()}; }

private:
    const std::vector<std::unique_ptr<PlotItem>>& items_;
};

/// Top-level plot container — owns axes, items, title, legend.
///
/// Given a widget size, computes the plot area rectangle accounting
/// for margins needed by axis labels, tick labels, and title.
class PlotScene {
public:
    PlotScene();

    // --- Item management (polymorphic) ---
    void addItem(std::unique_ptr<PlotItem> item);
    [[nodiscard]] const std::vector<std::unique_ptr<PlotItem>>& items() const { return items_; }
    [[nodiscard]] PlotItem* itemAt(std::size_t index);
    [[nodiscard]] const PlotItem* itemAt(std::size_t index) const;
    [[nodiscard]] std::size_t itemCount() const { return items_.size(); }
    void clearItems();

    // --- Backward-compatible series API ---
    void addSeries(LineSeries series);
    void clearSeries();
    [[nodiscard]] LineSeriesView series() const { return LineSeriesView(items_); }
    [[nodiscard]] std::size_t seriesCount() const { return items_.size(); }

    /// Mutable access to a LineSeries by index (for editing via commands).
    /// @throws std::out_of_range if index >= itemCount().
    LineSeries& seriesAt(std::size_t index);

    // --- Axes ---
    [[nodiscard]] Axis& xAxis() { return xAxis_; }
    [[nodiscard]] Axis& yAxis() { return yAxis_; }
    [[nodiscard]] const Axis& xAxis() const { return xAxis_; }
    [[nodiscard]] const Axis& yAxis() const { return yAxis_; }

    // --- View transform ---
    [[nodiscard]] ViewTransform& viewTransform() { return viewTransform_; }
    [[nodiscard]] const ViewTransform& viewTransform() const { return viewTransform_; }

    // --- Title ---
    void setTitle(const QString& title);
    [[nodiscard]] const QString& title() const { return title_; }

    /// Set the title font size in pixels (default: 17 from tokens::typography::title3).
    void setTitleFontPx(int px);
    /// Get the title font size in pixels.
    [[nodiscard]] int titleFontPx() const { return titleFontPx_; }

    /// Set the title font weight (default: QFont::DemiBold).
    void setTitleWeight(QFont::Weight w);
    /// Get the title font weight.
    [[nodiscard]] QFont::Weight titleWeight() const { return titleWeight_; }

    // --- Legend ---
    [[nodiscard]] Legend& legend() { return legend_; }
    [[nodiscard]] const Legend& legend() const { return legend_; }

    // --- Annotation layer (Phase 9) ---
    [[nodiscard]] AnnotationLayer* annotationLayer() { return &annotationLayer_; }
    [[nodiscard]] const AnnotationLayer* annotationLayer() const { return &annotationLayer_; }

    /// Compute the data plotting area given the total widget size.
    /// Accounts for margins: left (Y ticks+label), bottom (X ticks+label),
    /// top (title), right (padding).
    [[nodiscard]] QRectF computePlotArea(QSizeF widgetSize) const;

    /// Compute dynamic margins based on tick labels, axis labels, and title.
    /// Uses the actual font metrics to determine precise sizes.
    [[nodiscard]] PlotMargins computeMargins(const QFontMetrics& tickFm,
                                             const QFontMetrics& labelFm,
                                             const QFontMetrics& titleFm) const;

    /// Auto-range both axes from all item data and sync ViewTransform.
    void autoRange();

private:
    Axis xAxis_{AxisOrientation::Horizontal};
    Axis yAxis_{AxisOrientation::Vertical};
    ViewTransform viewTransform_;
    std::vector<std::unique_ptr<PlotItem>> items_;
    QString title_;
    int titleFontPx_ = 17;  // tokens::typography::title3.sizePx
    QFont::Weight titleWeight_ = QFont::DemiBold;
    Legend legend_;
    AnnotationLayer annotationLayer_;

    /// Cached margins for 1-pixel debounce (prevents jiggle during live edits).
    mutable PlotMargins cachedMargins_{};
    mutable bool hasCachedMargins_ = false;
};

}  // namespace lumen::plot
