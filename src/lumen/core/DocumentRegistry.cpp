#include "DocumentRegistry.h"
#include "EventBus.h"

#include <QDebug>

namespace lumen::core {

DocumentRegistry::DocumentRegistry(EventBus* eventBus, QObject* parent)
    : QObject(parent)
    , eventBus_(eventBus)
{
}

const lumen::data::TabularBundle* DocumentRegistry::addDocument(
    const QString& path, std::shared_ptr<lumen::data::TabularBundle> bundle)
{
    // If already open, return existing — no duplicate.
    auto it = documents_.find(path);
    if (it != documents_.end()) {
        qInfo() << "Document already open:" << path;
        return it->second.get();
    }

    auto [insertIt, ok] = documents_.emplace(path, std::move(bundle));
    Q_ASSERT(ok);

    qInfo() << "Document opened:" << path;

    Q_EMIT documentOpened(path);

    if (eventBus_ != nullptr) {
        eventBus_->publish(Event::DocumentOpened, QVariant(path));
    }

    return insertIt->second.get();
}

const lumen::data::TabularBundle* DocumentRegistry::document(const QString& path) const
{
    auto it = documents_.find(path);
    if (it == documents_.end()) {
        return nullptr;
    }
    return it->second.get();
}

bool DocumentRegistry::closeDocument(const QString& path)
{
    auto it = documents_.find(path);
    if (it == documents_.end()) {
        return false;
    }

    documents_.erase(it);
    qInfo() << "Document closed:" << path;

    Q_EMIT documentClosed(path);

    if (eventBus_ != nullptr) {
        eventBus_->publish(Event::DocumentClosed, QVariant(path));
    }

    return true;
}

int DocumentRegistry::count() const
{
    return static_cast<int>(documents_.size());
}

} // namespace lumen::core
