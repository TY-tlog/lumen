#pragma once

#include "CsvError.h"
#include "TabularBundle.h"

#include <QMetaType>
#include <QObject>
#include <QString>

#include <atomic>
#include <memory>

namespace lumen::data {

/// Asynchronous CSV file loader that runs CsvReader in a worker thread.
///
/// Usage:
///   auto* loader = new FileLoader(this);
///   connect(loader, &FileLoader::finished, this, [&](auto path, auto bundle) {
///       registry.addDocument(path, bundle);
///   });
///   loader->load("/path/to/file.csv");
///
/// The loader creates a QThread per load and destroys it on completion.
/// On success the finished signal carries a shared_ptr<TabularBundle>.
/// The caller is responsible for registering the result in a
/// DocumentRegistry if desired.
class FileLoader : public QObject {
    Q_OBJECT

public:
    /// Construct a loader.
    explicit FileLoader(QObject* parent = nullptr);

    ~FileLoader() override;

    /// Start an asynchronous load of @p filePath.
    /// Signals: progress, finished, or failed.
    void load(const QString& filePath);

    /// Request cancellation of the current load.
    void cancel();

    /// Whether a cancellation has been requested.
    [[nodiscard]] bool isCancelled() const;

signals:
    /// Emitted periodically during parsing (0..100).
    void progress(int percent);

    /// Emitted on successful load.
    void finished(const QString& path, std::shared_ptr<lumen::data::TabularBundle> result);

    /// Emitted on failure (parse error or file-not-found).
    void failed(const QString& path, const QString& errorMessage);

private:
    /// Runs in the worker thread.
    void doLoad(const QString& filePath);

    std::atomic<bool> cancelled_{false};
};

} // namespace lumen::data

Q_DECLARE_METATYPE(std::shared_ptr<lumen::data::TabularBundle>)
