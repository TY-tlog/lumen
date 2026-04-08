#include "core/PlotRegistry.h"

#include "core/EventBus.h"

namespace lumen::core {

PlotRegistry::PlotRegistry(EventBus* eventBus, QObject* parent)
    : QObject(parent)
    , eventBus_(eventBus) {}

void PlotRegistry::registerPlot(const QString& documentPath, QObject* canvas) {
    if (canvas == nullptr) {
        return;
    }

    if (plots_.contains(documentPath)) {
        auto* old = plots_.value(documentPath);
        disconnect(old, &QObject::destroyed, this, &PlotRegistry::onCanvasDestroyed);
    }

    plots_.insert(documentPath, canvas);
    connect(canvas, &QObject::destroyed, this, &PlotRegistry::onCanvasDestroyed);

    emit plotRegistered(documentPath);

    if (eventBus_ != nullptr) {
        eventBus_->publish(Event::PlotCreated, documentPath);
    }
}

void PlotRegistry::unregisterPlot(const QString& documentPath) {
    auto it = plots_.find(documentPath);
    if (it == plots_.end()) {
        return;
    }

    auto* canvas = it.value();
    disconnect(canvas, &QObject::destroyed, this, &PlotRegistry::onCanvasDestroyed);
    plots_.erase(it);

    emit plotUnregistered(documentPath);
}

QObject* PlotRegistry::plotFor(const QString& documentPath) const {
    return plots_.value(documentPath, nullptr);
}

void PlotRegistry::clear() {
    for (auto it = plots_.begin(); it != plots_.end(); ++it) {
        disconnect(it.value(), &QObject::destroyed, this, &PlotRegistry::onCanvasDestroyed);
    }
    plots_.clear();
}

int PlotRegistry::count() const {
    return static_cast<int>(plots_.size());
}

void PlotRegistry::onCanvasDestroyed(QObject* obj) {
    for (auto it = plots_.begin(); it != plots_.end(); ++it) {
        if (it.value() == obj) {
            QString path = it.key();
            plots_.erase(it);
            emit plotUnregistered(path);
            return;
        }
    }
}

}  // namespace lumen::core
