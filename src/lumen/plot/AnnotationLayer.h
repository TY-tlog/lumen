#pragma once

#include "Annotation.h"

#include <QList>
#include <QPoint>
#include <QRectF>

#include <memory>
#include <optional>
#include <vector>

class QPainter;

namespace lumen::plot {

class CoordinateMapper;

/// Container for annotations on a PlotScene.
///
/// Renders all annotations after PlotItems (overlay), provides
/// hit-testing for double-click editing, and serializes to JSON
/// for workspace persistence.
class AnnotationLayer {
public:
    struct HitResult {
        int annotationId;
        Annotation* annotation;
    };

    AnnotationLayer() = default;

    /// Add an annotation. Takes ownership. Returns its assigned ID.
    int addAnnotation(std::unique_ptr<Annotation> ann);

    /// Remove an annotation by ID. Returns true if found.
    bool removeAnnotation(int id);

    /// Get all annotations (non-owning pointers).
    [[nodiscard]] QList<Annotation*> all() const;

    /// Get annotation by ID.
    [[nodiscard]] Annotation* find(int id) const;

    /// Number of annotations.
    [[nodiscard]] int count() const;

    /// Paint all visible annotations.
    void paint(QPainter* painter, const CoordinateMapper& mapper,
               const QRectF& plotArea) const;

    /// Hit-test: find the top-most annotation containing the pixel.
    /// Returns nullopt if no annotation is hit.
    [[nodiscard]] std::optional<HitResult> hitTest(
        QPoint pixel, const CoordinateMapper& mapper,
        const QRectF& plotArea) const;

    /// Clear all annotations.
    void clear();

    /// Serialize all annotations to a JSON array.
    [[nodiscard]] QJsonArray toJsonArray() const;

    /// Deserialize annotations from a JSON array.
    void fromJsonArray(const QJsonArray& arr);

private:
    std::vector<std::unique_ptr<Annotation>> annotations_;
    int nextId_ = 1;
};

}  // namespace lumen::plot
