#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

#include <functional>
#include <mutex>
#include <vector>

namespace lumen::core {

/// Typed event categories for cross-module communication.
enum class Event {
    DocumentOpened,
    DocumentClosed,
    SelectionChanged,
    ThemeChanged,

    // Phase 2 plot events (added in Phase 2.5)
    PlotCreated,          // payload: QString documentPath
    PlotViewportChanged,  // payload: QString documentPath (future: QRectF viewport)
    PlotCrosshairMoved,   // payload: QString documentPath (future: QPointF dataPos)
    PlotColumnsChanged,   // payload: QString documentPath
};

/// Central event bus for decoupled cross-module communication.
///
/// Subscribers register a callback for a specific Event type.
/// Any module can emit an event with a QVariant payload; all
/// subscribers for that type are notified.  Thread-safe: emitting
/// from a worker thread delivers callbacks on the main thread via
/// queued connections.
class EventBus : public QObject {
    Q_OBJECT

public:
    /// Convenience type for subscriber callbacks.
    using Callback = std::function<void(const QVariant&)>;

    /// Construct an EventBus with an optional parent.
    explicit EventBus(QObject* parent = nullptr);

    /// Subscribe to an event type with a free callback.
    /// The caller is responsible for ensuring the callback remains
    /// valid for the lifetime of this EventBus (or until the bus
    /// is destroyed).
    void subscribe(Event type, const Callback& callback);

    /// Subscribe to an event type, automatically disconnecting when
    /// @p receiver is destroyed.
    void subscribe(Event type, QObject* receiver, const Callback& callback);

    /// Publish an event to all subscribers of the given type.
    /// Thread-safe: may be called from any thread.
    void publish(Event type, const QVariant& payload = {});

signals:
    /// Internal signal used to marshal cross-thread delivery.
    void eventPosted(int type, QVariant payload);

private slots:
    /// Dispatches the event to registered callbacks.
    void dispatchEvent(int type, const QVariant& payload);

private:
    struct Subscription {
        QObject* receiver = nullptr;  // may be nullptr for free callbacks
        Callback callback;
    };

    mutable std::mutex mutex_;
    std::vector<std::pair<Event, Subscription>> subscriptions_;
};

} // namespace lumen::core
