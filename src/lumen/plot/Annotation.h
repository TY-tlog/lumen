#pragma once

#include <QColor>
#include <QJsonObject>
#include <QPointF>
#include <QRectF>
#include <QString>

#include <memory>

class QPainter;

namespace lumen::plot {

class CoordinateMapper;

/// Abstract base for all annotation types rendered on a PlotScene.
///
/// Annotations overlay data and support three anchor modes:
/// - Data: positioned in data coordinates (moves with pan/zoom)
/// - Pixel: absolute pixel position (fixed on screen)
/// - AxisFraction: fractional (0..1) of the plot area
class Annotation {
public:
    enum class Type { Arrow, Box, Callout, Text, ScaleBar, ColorBar };
    enum class Anchor { Data, Pixel, AxisFraction };

    virtual ~Annotation() = default;

    [[nodiscard]] virtual Type type() const = 0;
    [[nodiscard]] virtual Anchor anchor() const { return anchor_; }

    /// Paint this annotation onto the given painter.
    virtual void paint(QPainter* painter, const CoordinateMapper& mapper,
                       const QRectF& plotArea) const = 0;

    /// Bounding rectangle in pixel coordinates (for hit-testing).
    [[nodiscard]] virtual QRectF boundingRect(const CoordinateMapper& mapper,
                                               const QRectF& plotArea) const = 0;

    /// Serialize to JSON for workspace persistence.
    [[nodiscard]] virtual QJsonObject toJson() const = 0;

    /// Create an Annotation from JSON (dispatches by "type" field).
    static std::unique_ptr<Annotation> fromJson(const QJsonObject& obj);

    // --- Common properties ---

    [[nodiscard]] int id() const { return id_; }
    void setId(int newId) { id_ = newId; }

    [[nodiscard]] bool isVisible() const { return visible_; }
    void setVisible(bool v) { visible_ = v; }

    [[nodiscard]] QString name() const { return name_; }
    void setName(const QString& n) { name_ = n; }

protected:
    Anchor anchor_ = Anchor::Data;
    int id_ = 0;
    bool visible_ = true;
    QString name_;

    /// Helper: resolve a point from data/pixel/fraction to pixel coords.
    [[nodiscard]] QPointF resolvePoint(QPointF point, Anchor anch,
                                        const CoordinateMapper& mapper,
                                        const QRectF& plotArea) const;
};

}  // namespace lumen::plot
