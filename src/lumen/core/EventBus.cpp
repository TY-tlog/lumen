#include "EventBus.h"

#include <QDebug>

#include <algorithm>

namespace lumen::core {

EventBus::EventBus(QObject* parent)
    : QObject(parent)
{
    // Queued connection so cross-thread emits are marshalled to
    // the thread that owns this EventBus (typically the main thread).
    connect(this, &EventBus::eventPosted, this, &EventBus::dispatchEvent,
            Qt::QueuedConnection);
}

void EventBus::subscribe(Event type, const Callback& callback)
{
    std::lock_guard lock(mutex_);
    subscriptions_.push_back({type, {nullptr, callback}});
}

void EventBus::subscribe(Event type, QObject* receiver, const Callback& callback)
{
    {
        std::lock_guard lock(mutex_);
        subscriptions_.push_back({type, {receiver, callback}});
    }

    // When the receiver is destroyed, remove its subscriptions.
    connect(receiver, &QObject::destroyed, this, [this, receiver]() {
        std::lock_guard lock(mutex_);
        std::erase_if(subscriptions_, [receiver](const auto& pair) {
            return pair.second.receiver == receiver;
        });
    });
}

void EventBus::publish(Event type, const QVariant& payload)
{
    // Fire the queued signal; dispatchEvent will run on this object's thread.
    Q_EMIT eventPosted(static_cast<int>(type), payload);
}

void EventBus::dispatchEvent(int type, const QVariant& payload)
{
    auto eventType = static_cast<Event>(type);

    // Snapshot the subscriptions under the lock, then call outside
    // the lock to avoid deadlocks if a callback subscribes/unsubscribes.
    std::vector<Callback> toCall;
    {
        std::lock_guard lock(mutex_);
        for (const auto& [evType, sub] : subscriptions_) {
            if (evType == eventType) {
                toCall.push_back(sub.callback);
            }
        }
    }

    for (const auto& cb : toCall) {
        cb(payload);
    }
}

} // namespace lumen::core
