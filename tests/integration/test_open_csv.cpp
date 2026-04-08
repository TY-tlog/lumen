// Integration tests: Open CSV end-to-end.
//
// These tests exercise the full data-loading path from file on disk
// through CsvReader, DataFrame, DocumentRegistry, and FileLoader.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <core/DocumentRegistry.h>
#include <core/EventBus.h>
#include <data/CsvError.h>
#include <data/CsvReader.h>
#include <data/DataFrame.h>
#include <data/FileLoader.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QString>

#include <cmath>
#include <memory>
#include <string>

using namespace lumen::data;
using namespace lumen::core;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Path to a test fixture file under tests/fixtures/tiny/.
static QString fixturePath(const char* name)
{
    return QStringLiteral(LUMEN_TEST_FIXTURES_DIR) + QStringLiteral("/tiny/")
        + QString::fromUtf8(name);
}

/// Ensure a QCoreApplication exists (needed for signals/slots and event loop).
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

/// Spin the event loop until a QSignalSpy receives at least one signal,
/// or timeout (ms) is reached.  Returns true if the signal was received.
static bool waitForSignal(QSignalSpy& spy, int timeoutMs = 5000)
{
    if (spy.count() > 0) {
        return true;
    }
    return spy.wait(timeoutMs);
}

// ---------------------------------------------------------------------------
// Test 1: Open electrophys CSV end-to-end
// ---------------------------------------------------------------------------

TEST_CASE("Open electrophys CSV end-to-end", "[integration]") {
    CsvReader reader;
    auto df = reader.readFile(fixturePath("electrophys_sample.csv"));

    // Verify dimensions
    REQUIRE(df.columnCount() == 9);
    REQUIRE(df.rowCount() == 20);

    // Verify column names
    REQUIRE(df.column(0).name() == QStringLiteral("time_ms"));
    REQUIRE(df.column(1).name() == QStringLiteral("voltage_mV"));
    REQUIRE(df.column(2).name() == QStringLiteral("current_stimulus_nA"));
    REQUIRE(df.column(3).name() == QStringLiteral("I_ion_nA"));
    REQUIRE(df.column(4).name() == QStringLiteral("dvdt_2pt_fwd"));
    REQUIRE(df.column(5).name() == QStringLiteral("dvdt_3pt_fwd"));
    REQUIRE(df.column(6).name() == QStringLiteral("dvdt_3pt_bwd"));
    REQUIRE(df.column(7).name() == QStringLiteral("dvdt_2pt_cen"));
    REQUIRE(df.column(8).name() == QStringLiteral("dvdt_5pt_cen"));

    // All columns should be Double
    for (std::size_t c = 0; c < df.columnCount(); ++c) {
        REQUIRE(df.column(c).type() == ColumnType::Double);
    }

    // Verify NaN values: dvdt_3pt_bwd at rows 0 and 1 should be NaN
    const auto& bwd = df.column(6).doubleData();
    REQUIRE(std::isnan(bwd[0]));
    REQUIRE(std::isnan(bwd[1]));

    // Verify a known value: row 0, time_ms should be 0.2
    REQUIRE(df.column(0).doubleData()[0] == 0.2);

    // Verify another known value: row 0, voltage_mV
    REQUIRE(df.column(1).doubleData()[0] == Catch::Approx(-38.226183));
}

// ---------------------------------------------------------------------------
// Test 2: DocumentRegistry stores loaded DataFrame
// ---------------------------------------------------------------------------

TEST_CASE("DocumentRegistry stores loaded DataFrame", "[integration]") {
    ensureApp();

    DocumentRegistry registry;
    CsvReader reader;

    auto df = reader.readFile(fixturePath("simple_3x4.csv"));
    REQUIRE(df.columnCount() == 3);
    REQUIRE(df.rowCount() == 4);

    auto dfPtr = std::make_shared<DataFrame>(std::move(df));
    QString path = fixturePath("simple_3x4.csv");

    // Add to registry
    const auto* stored = registry.addDocument(path, dfPtr);
    REQUIRE(stored != nullptr);

    // Retrieve by path
    const auto* retrieved = registry.document(path);
    REQUIRE(retrieved != nullptr);
    REQUIRE(retrieved->columnCount() == 3);
    REQUIRE(retrieved->rowCount() == 4);

    // Close and verify removal
    REQUIRE(registry.closeDocument(path));
    REQUIRE(registry.document(path) == nullptr);
}

// ---------------------------------------------------------------------------
// Test 3: FileLoader async load
// ---------------------------------------------------------------------------

TEST_CASE("FileLoader async load", "[integration]") {
    ensureApp();

    auto* loader = new FileLoader();
    QSignalSpy finishedSpy(loader, &FileLoader::finished);
    QSignalSpy failedSpy(loader, &FileLoader::failed);

    loader->load(fixturePath("electrophys_sample.csv"));

    REQUIRE(waitForSignal(finishedSpy));
    REQUIRE(failedSpy.count() == 0);
    REQUIRE(finishedSpy.count() == 1);

    // Verify the returned DataFrame has correct dimensions
    auto args = finishedSpy.takeFirst();
    auto dfPtr = args.at(1).value<std::shared_ptr<DataFrame>>();

    REQUIRE(dfPtr != nullptr);
    REQUIRE(dfPtr->columnCount() == 9);
    REQUIRE(dfPtr->rowCount() == 20);

    delete loader;
}

// ---------------------------------------------------------------------------
// Test 4: Error on malformed CSV
// ---------------------------------------------------------------------------

TEST_CASE("Error on malformed CSV", "[integration]") {
    CsvReader reader;

    // Nonexistent file should throw CsvError with a meaningful message
    try {
        auto df = reader.readFile(QStringLiteral("/no/such/path/nonexistent.csv"));
        static_cast<void>(df);
        REQUIRE(false); // Should not reach here
    } catch (const CsvError& e) {
        REQUIRE(std::string(e.what()).size() > 0);
        REQUIRE(!e.description().isEmpty());
    }
}

// ---------------------------------------------------------------------------
// Test 5: NaN handling end-to-end
// ---------------------------------------------------------------------------

TEST_CASE("NaN handling end-to-end", "[integration]") {
    CsvReader reader;
    auto df = reader.readFile(fixturePath("with_nan.csv"));

    REQUIRE(df.columnCount() == 3);
    REQUIRE(df.rowCount() == 4);

    // All columns should be Double (NaN values force numeric inference)
    for (std::size_t c = 0; c < df.columnCount(); ++c) {
        REQUIRE(df.column(c).type() == ColumnType::Double);
    }

    // Row 0: x=1.0, y=NaN, z=3.0
    REQUIRE(df.column(0).doubleData()[0] == 1.0);
    REQUIRE(std::isnan(df.column(1).doubleData()[0]));
    REQUIRE(df.column(2).doubleData()[0] == 3.0);

    // Row 1: x=2.0, y=4.0, z=(empty -> NaN)
    REQUIRE(df.column(0).doubleData()[1] == 2.0);
    REQUIRE(df.column(1).doubleData()[1] == 4.0);
    REQUIRE(std::isnan(df.column(2).doubleData()[1]));

    // Row 2: x=NaN, y=6.0, z=7.0
    REQUIRE(std::isnan(df.column(0).doubleData()[2]));
    REQUIRE(df.column(1).doubleData()[2] == 6.0);
    REQUIRE(df.column(2).doubleData()[2] == 7.0);

    // Row 3: x=4.0, y=(empty -> NaN), z=NaN
    REQUIRE(df.column(0).doubleData()[3] == 4.0);
    REQUIRE(std::isnan(df.column(1).doubleData()[3]));
    REQUIRE(std::isnan(df.column(2).doubleData()[3]));

    // Verify that NaN values are proper std::isnan (quiet NaN)
    double nanVal = df.column(1).doubleData()[0];
    REQUIRE(std::isnan(nanVal));
    REQUIRE(nanVal != nanVal); // NaN is not equal to itself
}
