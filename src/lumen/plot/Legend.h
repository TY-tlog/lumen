#pragma once

#include <QObject>

namespace lumen::plot {

/// Position of the legend relative to the plot area.
enum class LegendPosition {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    OutsideRight
};

/// Legend state for a plot — owns position and visibility.
///
/// Does NOT own series data; PlotRenderer reads entries from
/// PlotScene::series() at render time (name + color from each LineSeries).
class Legend : public QObject {
    Q_OBJECT
public:
    explicit Legend(QObject* parent = nullptr);

    /// Set the legend position.
    void setPosition(LegendPosition pos);

    /// Current legend position.
    [[nodiscard]] LegendPosition position() const;

    /// Set whether the legend is visible.
    void setVisible(bool visible);

    /// Whether the legend is visible.
    [[nodiscard]] bool isVisible() const;

signals:
    /// Emitted when position or visibility changes.
    void changed();

private:
    LegendPosition position_ = LegendPosition::TopRight;
    bool visible_ = true;
};

}  // namespace lumen::plot
