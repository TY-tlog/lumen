// Unit tests for CsvLoader (wraps CsvReader through DatasetLoader interface).

#include <catch2/catch_test_macros.hpp>

#include <data/io/CsvLoader.h>
#include <data/io/DatasetLoader.h>
#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>

#include <QString>

#include <memory>

using namespace lumen::data;
using namespace lumen::data::io;

static QString fixturePath(const char* name)
{
    return QStringLiteral(LUMEN_TEST_FIXTURES_DIR) + QStringLiteral("/tiny/") + QString::fromUtf8(name);
}

TEST_CASE("CsvLoader -- supported extensions", "[csv_loader]") {
    CsvLoader loader;
    QStringList exts = loader.supportedExtensions();
    REQUIRE(exts.contains(QStringLiteral("csv")));
    REQUIRE(exts.contains(QStringLiteral("tsv")));
}

TEST_CASE("CsvLoader -- canLoad recognizes .csv", "[csv_loader]") {
    CsvLoader loader;
    REQUIRE(loader.canLoad(QStringLiteral("/tmp/test.csv")));
    REQUIRE(loader.canLoad(QStringLiteral("/tmp/test.CSV")));
    REQUIRE(loader.canLoad(QStringLiteral("/tmp/test.tsv")));
    REQUIRE_FALSE(loader.canLoad(QStringLiteral("/tmp/test.h5")));
}

TEST_CASE("CsvLoader -- loadTabular reads simple CSV", "[csv_loader]") {
    CsvLoader loader;
    auto bundle = loader.loadTabular(fixturePath("simple_3x4.csv"));

    REQUIRE(bundle != nullptr);
    REQUIRE(bundle->columnCount() == 3);
    REQUIRE(bundle->rowCount() == 4);
    REQUIRE(bundle->column(0)->name() == QStringLiteral("a"));
    REQUIRE(bundle->column(0)->doubleData()[0] == 1.0);
}

TEST_CASE("CsvLoader -- interface polymorphism", "[csv_loader]") {
    std::unique_ptr<DatasetLoader> loader = std::make_unique<CsvLoader>();
    REQUIRE(loader->canLoad(QStringLiteral("/tmp/test.csv")));
    auto bundle = loader->loadTabular(fixturePath("simple_3x4.csv"));
    REQUIRE(bundle != nullptr);
    REQUIRE(bundle->columnCount() == 3);
}
