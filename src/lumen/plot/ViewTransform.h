#pragma once

namespace lumen::plot {

/// Stores a base data range and a current (transformed) view range.
///
/// Supports pan and zoom operations on the view, with reset to the base range.
class ViewTransform {
public:
    ViewTransform() = default;

    /// Construct with an initial base range.
    ViewTransform(double xMin, double xMax, double yMin, double yMax);

    /// Set the base range and reset the current view to match.
    void setBaseRange(double xMin, double xMax, double yMin, double yMax);

    /// Pan the view by the given data-space offsets.
    void pan(double dxData, double dyData);

    /// Zoom both axes by the given factor, centered on (centerX, centerY) in data space.
    /// factor > 1 zooms in (narrower range), factor < 1 zooms out.
    void zoom(double factor, double centerX, double centerY);

    /// Zoom X axis only.
    void zoomX(double factor, double centerX);

    /// Zoom Y axis only.
    void zoomY(double factor, double centerY);

    /// Reset the current view to the base range.
    void reset();

    [[nodiscard]] double xMin() const { return viewXMin_; }
    [[nodiscard]] double xMax() const { return viewXMax_; }
    [[nodiscard]] double yMin() const { return viewYMin_; }
    [[nodiscard]] double yMax() const { return viewYMax_; }

private:
    double baseXMin_{0.0};
    double baseXMax_{1.0};
    double baseYMin_{0.0};
    double baseYMax_{1.0};

    double viewXMin_{0.0};
    double viewXMax_{1.0};
    double viewYMin_{0.0};
    double viewYMax_{1.0};
};

} // namespace lumen::plot
