#include "FileLoader.h"
#include "CsvReader.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QThread>

#include <memory>

namespace lumen::data {

FileLoader::FileLoader(QObject* parent)
    : QObject(parent)
{
}

FileLoader::~FileLoader() = default;

void FileLoader::load(const QString& filePath)
{
    cancelled_.store(false, std::memory_order_relaxed);

    // Create a worker thread for this load.
    auto* thread = new QThread(this);

    connect(thread, &QThread::started, this, [this, filePath]() { doLoad(filePath); },
            Qt::DirectConnection);

    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    // Move this loader to the worker thread so doLoad runs there.
    this->moveToThread(thread);
    thread->start();
}

void FileLoader::cancel()
{
    cancelled_.store(true, std::memory_order_relaxed);
}

bool FileLoader::isCancelled() const
{
    return cancelled_.load(std::memory_order_relaxed);
}

void FileLoader::doLoad(const QString& filePath)
{
    auto moveBackAndQuit = [this]() {
        QThread* workerThread = QThread::currentThread();
        QThread* mainThread = QCoreApplication::instance() != nullptr
                                  ? QCoreApplication::instance()->thread()
                                  : workerThread;
        this->moveToThread(mainThread);
        workerThread->quit();
    };

    Q_EMIT progress(0);

    // Check file exists.
    if (!QFile::exists(filePath)) {
        Q_EMIT failed(filePath, QStringLiteral("File not found: ") + filePath);
        moveBackAndQuit();
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        Q_EMIT failed(filePath, QStringLiteral("Cannot open file: ") + file.errorString());
        moveBackAndQuit();
        return;
    }

    Q_EMIT progress(10);

    if (cancelled_.load(std::memory_order_relaxed)) {
        Q_EMIT failed(filePath, QStringLiteral("Load cancelled"));
        moveBackAndQuit();
        return;
    }

    QByteArray bytes = file.readAll();
    file.close();

    Q_EMIT progress(30);

    if (cancelled_.load(std::memory_order_relaxed)) {
        Q_EMIT failed(filePath, QStringLiteral("Load cancelled"));
        moveBackAndQuit();
        return;
    }

    try {
        std::string content(bytes.constData(), static_cast<std::size_t>(bytes.size()));
        bytes.clear();  // Free memory early.

        Q_EMIT progress(40);

        CsvReader reader;
        TabularBundle bundle = reader.readString(content);

        if (cancelled_.load(std::memory_order_relaxed)) {
            Q_EMIT failed(filePath, QStringLiteral("Load cancelled"));
            moveBackAndQuit();
            return;
        }

        Q_EMIT progress(90);

        auto sharedBundle = std::make_shared<TabularBundle>(std::move(bundle));

        Q_EMIT progress(100);
        Q_EMIT finished(filePath, std::move(sharedBundle));
    } catch (const CsvError& e) {
        Q_EMIT failed(filePath, QString::fromStdString(e.what()));
    } catch (const std::exception& e) {
        Q_EMIT failed(filePath, QString::fromStdString(e.what()));
    }

    moveBackAndQuit();
}

} // namespace lumen::data
