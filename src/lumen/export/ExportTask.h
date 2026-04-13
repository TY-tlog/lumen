#pragma once

#include <core/io/FigureExporter.h>

#include <QObject>
#include <QString>

#include <atomic>
#include <memory>

class QThread;

namespace lumen::plot {
class PlotScene;
}

namespace lumen::exp {

/// Asynchronous export task that renders a PlotScene in a background
/// QThread with progress reporting and cooperative cancellation.
///
/// Usage:
///   auto task = new ExportTask(opts, this);
///   connect(task, &ExportTask::progress, ...);
///   connect(task, &ExportTask::finished, ...);
///   task->start();
///
/// Cancel: call cancel(). The render loop checks between PlotItems
/// and aborts early. No output file is produced on cancel.
///
/// Atomic write: renders to a temp file, renames on success.
class ExportTask : public QObject {
    Q_OBJECT

public:
    struct Options {
        core::io::FigureExporter::Options exportOpts;
        const plot::PlotScene* scene = nullptr;
    };

    explicit ExportTask(Options opts, QObject* parent = nullptr);
    ~ExportTask() override;

    /// Start the export in a background thread.
    void start();

    /// Request cooperative cancellation.
    void cancel();

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] bool isCancelRequested() const;

signals:
    void progress(int percent);
    void finished(bool success, const QString& outputPath);
    void error(const QString& message);

private:
    void doExport();

    Options opts_;
    QThread* thread_ = nullptr;
    std::atomic<bool> cancelRequested_{false};
    std::atomic<bool> running_{false};
};

}  // namespace lumen::exp
