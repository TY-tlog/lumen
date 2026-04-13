#include "AnnotationLayer.h"

#include <QJsonArray>
#include <QPainter>

namespace lumen::plot {

int AnnotationLayer::addAnnotation(std::unique_ptr<Annotation> ann)
{
    int id = nextId_++;
    ann->setId(id);
    annotations_.push_back(std::move(ann));
    return id;
}

bool AnnotationLayer::removeAnnotation(int id)
{
    auto it = std::find_if(annotations_.begin(), annotations_.end(),
                           [id](const auto& a) { return a->id() == id; });
    if (it == annotations_.end())
        return false;
    annotations_.erase(it);
    return true;
}

QList<Annotation*> AnnotationLayer::all() const
{
    QList<Annotation*> result;
    result.reserve(static_cast<int>(annotations_.size()));
    for (const auto& a : annotations_)
        result.append(a.get());
    return result;
}

Annotation* AnnotationLayer::find(int id) const
{
    for (const auto& a : annotations_) {
        if (a->id() == id)
            return a.get();
    }
    return nullptr;
}

int AnnotationLayer::count() const
{
    return static_cast<int>(annotations_.size());
}

void AnnotationLayer::paint(QPainter* painter, const CoordinateMapper& mapper,
                             const QRectF& plotArea) const
{
    for (const auto& ann : annotations_) {
        if (ann->isVisible())
            ann->paint(painter, mapper, plotArea);
    }
}

std::optional<AnnotationLayer::HitResult> AnnotationLayer::hitTest(
    QPoint pixel, const CoordinateMapper& mapper,
    const QRectF& plotArea) const
{
    // Iterate in reverse (top-most painted last, hit first).
    for (auto it = annotations_.rbegin(); it != annotations_.rend(); ++it) {
        const auto& ann = *it;
        if (!ann->isVisible())
            continue;
        QRectF bounds = ann->boundingRect(mapper, plotArea);
        if (bounds.contains(pixel)) {
            return HitResult{ann->id(), ann.get()};
        }
    }
    return std::nullopt;
}

void AnnotationLayer::clear()
{
    annotations_.clear();
    nextId_ = 1;
}

QJsonArray AnnotationLayer::toJsonArray() const
{
    QJsonArray arr;
    for (const auto& ann : annotations_)
        arr.append(ann->toJson());
    return arr;
}

void AnnotationLayer::fromJsonArray(const QJsonArray& arr)
{
    clear();
    for (const auto& val : arr) {
        auto ann = Annotation::fromJson(val.toObject());
        if (ann)
            addAnnotation(std::move(ann));
    }
}

}  // namespace lumen::plot
