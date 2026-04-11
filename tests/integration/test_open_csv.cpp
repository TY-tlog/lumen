// Integration tests: Open CSV end-to-end.
//
// These tests exercise the full data-loading path from file on disk
// through CsvReader, TabularBundle, DocumentRegistry, and FileLoader.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <core/DocumentRegistry.h>
#include <core/EventBus.h>
#include <data/CsvError.h>
#include <data/CsvReader.h>
#include <data/FileLoader.h>
#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>
#include <data/Unit.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QString>

#include <cmath>
#include <memory>
#include <string>

using namespace lumen::data;
using namespace lumen::core;

static QString fixturePath(const char* name)
{
    return QStringLiteral(LUMEN_TEST_FIXTURES_DIR) + QStringLiteral("/tiny/")
        + QString::fromUtf8(name);
}

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

TEST_CASE("Open electrophys CSV end-to-end", "[integration]") {
    CsvReader reader;
    auto df = reader.readFile(fixturePath("electrophys_sample.csv"));

    REQUIRE(df.columnCount() == 9);
    REQUIRE(df.rowCount() == 20);

    REQUIRE(df.column(0)->name() == QStringLiteral("time_ms"));
    REQUIRE(df.column(1)->name() == QStringLiteral("voltage_mV"));
    REQUIRE(df.column(2)->name() == QStringLiteral("current_stimulus_nA"));
    REQUIRE(df.column(3)->name() == QStringLiteral("I_ion_nA"));
    REQUIRE(df.column(4)->name() == QStringLiteral("dvdt_2pt_fwd"));
    REQUIRE(df.column(5)->name() == QStringLiteral("dvdt_3pt_fwd"));
    REQUIRE(df.column(6)->name() == QStringLiteral("dvdt_3pt_bwd"));
    REQUIRE(df.column(7)->name() == QStringLiteral("dvdt_2pt_cen"));
    REQUIRE(df.column(8)->name() == QStringLiteral("dvdt_5pt_cen"));

    for (int c = 0; c < df.columnCount(); ++c) {
        CHECK_NOTHROW(df.column(c)->doubleData());
    }

    const auto& bwd = df.column(6)->doubleData();
    REQUIRE(std::isnan(bwd[0]));
    REQUIRE(std::isnan(bwd[1]));

    REQUIRE(df.column(0)->doubleData()[0] == 0.2);
    REQUIRE(df.column(1)->doubleData()[0] == Catch::Approx(-38.226183));
}

TEST_CASE("DocumentRegistry stores loaded TabularBundle", "[integration]") {
    ensureApp();

    DocumentRegistry registry;
    CsvReader reader;

    auto df = reader.readFile(fixturePath("simple_3x4.csv"));
    REQUIRE(df.columnCount() == 3);
    REQUIRE(df.rowCount() == 4);

    auto bundlePtr = std::make_shared<TabularBundle>(std::move(df));
    QString path = fixturePath("simple_3x4.csv");

    const auto* stored = registry.addDocument(path, bundlePtr);
    REQUIRE(stored != nullptr);

    const auto* retrieved = registry.document(path);
    REQUIRE(retrieved != nullptr);
    REQUIRE(retrieved->columnCount() == 3);
    REQUIRE(retrieved->rowCount() == 4);

    REQUIRE(registry.closeDocument(path));
    REQUIRE(registry.document(path) == nullptr);
}

TEST_CASE("FileLoader async load", "[integration]") {
    ensureApp();

    auto* loader = new FileLoader();
    QSignalSpy finishedSpy(loader, &FileLoader::finished);
    QSignalSpy failedSpy(loader, &FileLoader::failed);

    loader->load(fixturePath("electrophys_sample.csv"));

    REQUIRE(waitForSignal(finishedSpy));
    REQUIRE(failedSpy.count() == 0);
    REQUIRE(finishedSpy.count() == 1);

    auto args = finishedSpy.takeFirst();
    auto bundlePtr = args.at(1).value<std::shared_ptr<TabularBundle>>();

    REQUIRE(bundlePtr != nullptr);
    REQUIRE(bundlePtr->columnCount() == 9);
    REQUIRE(bundlePtr->rowCount() == 20);

    delete loader;
}

TEST_CASE("Error on malformed CSV", "[integration]") {
    CsvReader reader;

    try {
        auto df = reader.readFile(QStringLiteral("/no/such/path/nonexistent.csv"));
        static_cast<void>(df);
        REQUIRE(false);
    } catch (const CsvError& e) {
        REQUIRE(std::string(e.what()).size() > 0);
        REQUIRE(!e.description().isEmpty());
    }
}

TEST_CASE("NaN handling end-to-end", "[integration]") {
    CsvReader reader;
    auto df = reader.readFile(fixturePath("with_nan.csv"));

    REQUIRE(df.columnCount() == 3);
    REQUIRE(df.rowCount() == 4);

    for (int c = 0; c < df.columnCount(); ++c) {
        CHECK_NOTHROW(df.column(c)->doubleData());
    }

    REQUIRE(df.column(0)->doubleData()[0] == 1.0);
    REQUIRE(std::isnan(df.column(1)->doubleData()[0]));
    REQUIRE(df.column(2)->doubleData()[0] == 3.0);

    REQUIRE(df.column(0)->doubleData()[1] == 2.0);
    REQUIRE(df.column(1)->doubleData()[1] == 4.0);
    REQUIRE(std::isnan(df.column(2)->doubleData()[1]));

    REQUIRE(std::isnan(df.column(0)->doubleData()[2]));
    REQUIRE(df.column(1)->doubleData()[2] == 6.0);
    REQUIRE(df.column(2)->doubleData()[2] == 7.0);

    REQUIRE(df.column(0)->doubleData()[3] == 4.0);
    REQUIRE(std::isnan(df.column(1)->doubleData()[3]));
    REQUIRE(std::isnan(df.column(2)->doubleData()[3]));

    double nanVal = df.column(1)->doubleData()[0];
    REQUIRE(std::isnan(nanVal));
    REQUIRE(nanVal != nanVal);
}
