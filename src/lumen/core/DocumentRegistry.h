#pragma once

#include <data/TabularBundle.h>

#include <QObject>
#include <QString>

#include <memory>
#include <unordered_map>

namespace lumen::core {

class EventBus;

/// Registry of open documents (TabularBundles) keyed by file path.
///
/// Owns the TabularBundle instances via shared_ptr (shared_ptr is used
/// because TabularBundles may also be referenced by UI models while
/// still registered here, and because they need to be passed through
/// Qt signals).
///
/// Emits Qt signals and EventBus events on open/close.
class DocumentRegistry : public QObject {
    Q_OBJECT

public:
    /// Construct a registry.  If @p eventBus is non-null, document
    /// lifecycle events are also published there.
    explicit DocumentRegistry(EventBus* eventBus = nullptr, QObject* parent = nullptr);

    /// Add (or replace) a document.  If a document with the same
    /// path already exists, the existing one is returned and the
    /// new TabularBundle is discarded.
    /// Returns a raw non-owning pointer to the stored TabularBundle.
    const lumen::data::TabularBundle* addDocument(const QString& path,
                                                  std::shared_ptr<lumen::data::TabularBundle> bundle);

    /// Retrieve a document by path.  Returns nullptr if not found.
    [[nodiscard]] const lumen::data::TabularBundle* document(const QString& path) const;

    /// Close (remove) a document.  Returns true if it was found and removed.
    bool closeDocument(const QString& path);

    /// Number of open documents.
    [[nodiscard]] int count() const;

signals:
    /// Emitted after a new document is registered.
    void documentOpened(const QString& path);

    /// Emitted after a document is removed.
    void documentClosed(const QString& path);

private:
    struct QStringHash {
        std::size_t operator()(const QString& s) const { return qHash(s); }
    };

    EventBus* eventBus_ = nullptr;
    std::unordered_map<QString, std::shared_ptr<lumen::data::TabularBundle>, QStringHash> documents_;
};

} // namespace lumen::core
