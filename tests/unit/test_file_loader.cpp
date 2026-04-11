// Unit tests for FileLoader.

#include <catch2/catch_test_macros.hpp>

#include <data/FileLoader.h>
#include <data/TabularBundle.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QString>

#include <memory>

using namespace lumen::data;

static QCoreApplication* ensureApp()
{
    if (QCoreApplication::instance() == nullptr) {
        static int argc = 1;
        static const char* argv[] = {"test"};
        static auto* app = new QCoreApplication(argc, const_cast<char**>(argv));
        return app;
    }
    return QCoreApplication::instance();
}

static bool waitForSignal(QSignalSpy& spy, int timeoutMs = 5000)
{
    if (spy.count() > 0) {
        return true;
    }
    return spy.wait(timeoutMs);
}

TEST_CASE("FileLoader loads a small CSV file asynchronously", "[fileloader]") {
    ensureApp();

    auto* loader = new FileLoader();
    QSignalSpy finishedSpy(loader, &FileLoader::finished);
    QSignalSpy progressSpy(loader, &FileLoader::progress);
    QSignalSpy failedSpy(loader, &FileLoader::failed);

    QString fixturePath =
        QStringLiteral(LUMEN_TEST_FIXTURES_DIR) + QStringLiteral("/tiny/simple_3x4.csv");

    loader->load(fixturePath);

    REQUIRE(waitForSignal(finishedSpy));
    REQUIRE(failedSpy.count() == 0);
    REQUIRE(finishedSpy.count() == 1);

    // Verify the returned TabularBundle.
    auto args = finishedSpy.takeFirst();
    auto path = args.at(0).toString();
    auto bundlePtr = args.at(1).value<std::shared_ptr<TabularBundle>>();

    REQUIRE(path == fixturePath);
    REQUIRE(bundlePtr != nullptr);
    REQUIRE(bundlePtr->columnCount() == 3);
    REQUIRE(bundlePtr->rowCount() == 4);

    delete loader;
}

TEST_CASE("FileLoader emits progress signals", "[fileloader]") {
    ensureApp();

    auto* loader = new FileLoader();
    QSignalSpy progressSpy(loader, &FileLoader::progress);
    QSignalSpy finishedSpy(loader, &FileLoader::finished);

    QString fixturePath =
        QStringLiteral(LUMEN_TEST_FIXTURES_DIR) + QStringLiteral("/tiny/simple_3x4.csv");

    loader->load(fixturePath);
    REQUIRE(waitForSignal(finishedSpy));

    REQUIRE(progressSpy.count() >= 2);

    delete loader;
}

TEST_CASE("FileLoader emits failed for nonexistent file", "[fileloader]") {
    ensureApp();

    auto* loader = new FileLoader();
    QSignalSpy failedSpy(loader, &FileLoader::failed);
    QSignalSpy finishedSpy(loader, &FileLoader::finished);

    loader->load(QStringLiteral("/no/such/file/does_not_exist.csv"));

    REQUIRE(waitForSignal(failedSpy));
    REQUIRE(finishedSpy.count() == 0);
    REQUIRE(failedSpy.count() == 1);

    auto args = failedSpy.takeFirst();
    REQUIRE(args.at(1).toString().contains(QStringLiteral("not found")));

    delete loader;
}
