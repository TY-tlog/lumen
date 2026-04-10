#include "plot/PlotScene.h"

#include "style/DesignTokens.h"

#include <QFont>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace lumen::plot {

PlotScene::PlotScene() = default;

void PlotScene::addItem(std::unique_ptr<PlotItem> item) {
    items_.push_back(std::move(item));
}

PlotItem* PlotScene::itemAt(std::size_t index) {
    if (index >= items_.size()) {
        throw std::out_of_range("PlotScene::itemAt: index out of range");
    }
    return items_[index].get();
}

const PlotItem* PlotScene::itemAt(std::size_t index) const {
    if (index >= items_.size()) {
        throw std::out_of_range("PlotScene::itemAt: index out of range");
    }
    return items_[index].get();
}

void PlotScene::clearItems() {
    items_.clear();
}

void PlotScene::addSeries(LineSeries series) {
    items_.push_back(std::make_unique<LineSeries>(std::move(series)));
}

void PlotScene::clearSeries() {
    clearItems();
}

LineSeries& PlotScene::seriesAt(std::size_t index) {
    if (index >= items_.size()) {
        throw std::out_of_range("PlotScene::seriesAt: index out of range");
    }
    auto* ls = dynamic_cast<LineSeries*>(items_[index].get());
    assert(ls && "PlotScene::seriesAt: item is not a LineSeries");
    return *ls;
}

void PlotScene::setTitle(const QString& title) {
    title_ = title;
}

void PlotScene::setTitleFontPx(int px) {
    titleFontPx_ = px;
}

void PlotScene::setTitleWeight(QFont::Weight w) {
    titleWeight_ = w;
}

PlotMargins PlotScene::computeMargins(const QFontMetrics& tickFm,
                                       const QFontMetrics& labelFm,
                                       const QFontMetrics& titleFm) const
{
    PlotMargins margins;

    // Left margin: max Y tick label width + spacing + Y axis label height (if present) + spacing.
    {
        int maxTickWidth = 0;
        auto yTicks = yAxis_.ticks();
        for (const auto& tick : yTicks) {
            int w = tickFm.horizontalAdvance(tick.label);
            maxTickWidth = std::max(maxTickWidth, w);
        }
        margins.left = maxTickWidth + tokens::spacing::md;
        if (!yAxis_.label().isEmpty()) {
            margins.left += labelFm.height() + tokens::spacing::sm;
        }
    }

    // Bottom margin: X tick label height + spacing + X axis label height (if present) + spacing.
    {
        margins.bottom = tickFm.height() + tokens::spacing::md;
        if (!xAxis_.label().isEmpty()) {
            margins.bottom += labelFm.height() + tokens::spacing::sm;
        }
    }

    // Top margin: title present ? titleFm.height() + spacing : small padding.
    {
        if (!title_.isEmpty()) {
            margins.top = titleFm.height() + tokens::spacing::md;
        } else {
            margins.top = tokens::spacing::sm;
        }
    }

    // Right margin: base padding (OutsideRight legend support added later).
    margins.right = tokens::spacing::md;

    // 1-pixel debounce: if cached margins differ by at most 1px in all dimensions,
    // reuse cached values to prevent jiggle during live edits.
    if (hasCachedMargins_) {
        bool withinThreshold =
            std::abs(margins.left - cachedMargins_.left) <= 1.0 &&
            std::abs(margins.top - cachedMargins_.top) <= 1.0 &&
            std::abs(margins.right - cachedMargins_.right) <= 1.0 &&
            std::abs(margins.bottom - cachedMargins_.bottom) <= 1.0;
        if (withinThreshold) {
            return cachedMargins_;
        }
    }

    cachedMargins_ = margins;
    hasCachedMargins_ = true;
    return margins;
}

QRectF PlotScene::computePlotArea(QSizeF widgetSize) const {
    // Create default font metrics for tick, label, and title fonts.
    QFont tickFont;
    tickFont.setPixelSize(tokens::typography::footnote.sizePx);
    tickFont.setWeight(tokens::typography::footnote.weight);

    QFont labelFont;
    labelFont.setPixelSize(tokens::typography::bodyStrong.sizePx);
    labelFont.setWeight(tokens::typography::bodyStrong.weight);

    QFont titleFont;
    titleFont.setPixelSize(tokens::typography::title3.sizePx);
    titleFont.setWeight(tokens::typography::title3.weight);

    QFontMetrics tickFm(tickFont);
    QFontMetrics labelFm(labelFont);
    QFontMetrics titleFm(titleFont);

    auto margins = computeMargins(tickFm, labelFm, titleFm);

    double x = margins.left;
    double y = margins.top;
    double w = widgetSize.width() - margins.left - margins.right;
    double h = widgetSize.height() - margins.top - margins.bottom;

    w = std::max(w, 1.0);
    h = std::max(h, 1.0);

    return {x, y, w, h};
}

void PlotScene::autoRange() {
    // Build a temporary vector of LineSeries for Axis::autoRange compatibility.
    std::vector<LineSeries> seriesVec;
    for (const auto& item : items_) {
        if (auto* ls = dynamic_cast<const LineSeries*>(item.get())) {
            seriesVec.push_back(*ls);
        }
    }
    xAxis_.autoRange(seriesVec);
    yAxis_.autoRange(seriesVec);
    viewTransform_.setBaseRange(xAxis_.min(), xAxis_.max(),
                                yAxis_.min(), yAxis_.max());
}

}  // namespace lumen::plot
