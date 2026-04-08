#pragma once

#include <QHash>
#include <QObject>
#include <QString>

namespace lumen::core {

class EventBus;

/// Registry mapping document paths to their associated plot canvas objects.
///
/// Non-owning: stores raw QObject pointers. Connects to QObject::destroyed
/// for automatic cleanup when a canvas is deleted. Publishes
/// EventBus::PlotCreated on registration.
///
/// Uses QObject* (not QWidget*) so that core/ does not depend on Qt6::Widgets.
class PlotRegistry : public QObject {
    Q_OBJECT

public:
    explicit PlotRegistry(EventBus* eventBus = nullptr, QObject* parent = nullptr);

    /// Register a plot canvas for a document path.
    /// Overwrites any existing registration for the same path.
    void registerPlot(const QString& documentPath, QObject* canvas);

    /// Unregister a plot canvas by document path.
    void unregisterPlot(const QString& documentPath);

    /// Look up the canvas for a document path. Returns nullptr if not found.
    [[nodiscard]] QObject* plotFor(const QString& documentPath) const;

    /// Remove all registrations.
    void clear();

    /// Number of registered plots.
    [[nodiscard]] int count() const;

signals:
    void plotRegistered(const QString& documentPath);
    void plotUnregistered(const QString& documentPath);

private:
    void onCanvasDestroyed(QObject* obj);

    EventBus* eventBus_ = nullptr;
    QHash<QString, QObject*> plots_;
};

}  // namespace lumen::core
