#include "ExportTask.h"

#include <core/io/FigureExporter.h>
#include <plot/PlotScene.h>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QThread>

namespace lumen::exp {

ExportTask::ExportTask(Options opts, QObject* parent)
    : QObject(parent)
    , opts_(std::move(opts))
{
}

ExportTask::~ExportTask()
{
    if (thread_ != nullptr) {
        thread_->quit();
        thread_->wait();
    }
}

void ExportTask::start()
{
    if (running_.load())
        return;

    running_.store(true);
    cancelRequested_.store(false);

    thread_ = new QThread(this);
    connect(thread_, &QThread::started, this, &ExportTask::doExport);
    connect(thread_, &QThread::finished, thread_, &QObject::deleteLater);
    thread_->start();
}

void ExportTask::cancel()
{
    cancelRequested_.store(true);
}

bool ExportTask::isRunning() const
{
    return running_.load();
}

bool ExportTask::isCancelRequested() const
{
    return cancelRequested_.load();
}

void ExportTask::doExport()
{
    emit progress(0);

    if (opts_.scene == nullptr) {
        running_.store(false);
        emit error(QStringLiteral("No scene to export"));
        emit finished(false, {});
        return;
    }

    // Write to temp file first (atomic write pattern).
    QString finalPath = opts_.exportOpts.outputPath;
    QString tempPath = finalPath + QStringLiteral(".lumen_tmp");

    emit progress(10);

    if (cancelRequested_.load()) {
        running_.store(false);
        emit finished(false, {});
        return;
    }

    // Use the existing synchronous FigureExporter with temp path.
    auto tempOpts = opts_.exportOpts;
    tempOpts.outputPath = tempPath;

    emit progress(30);

    QString err = core::io::FigureExporter::exportFigure(opts_.scene, tempOpts);

    if (cancelRequested_.load()) {
        QFile::remove(tempPath);
        running_.store(false);
        emit finished(false, {});
        return;
    }

    emit progress(80);

    if (!err.isEmpty()) {
        QFile::remove(tempPath);
        running_.store(false);
        emit error(err);
        emit finished(false, {});
        return;
    }

    // Atomic rename: remove target first if it exists.
    if (QFile::exists(finalPath))
        QFile::remove(finalPath);

    if (!QFile::rename(tempPath, finalPath)) {
        QFile::remove(tempPath);
        running_.store(false);
        emit error(QStringLiteral("Failed to rename temp file to %1").arg(finalPath));
        emit finished(false, {});
        return;
    }

    emit progress(100);
    running_.store(false);
    emit finished(true, finalPath);
}

}  // namespace lumen::exp
