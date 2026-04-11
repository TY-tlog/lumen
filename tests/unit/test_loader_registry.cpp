// Unit tests for LoaderRegistry.

#include <catch2/catch_test_macros.hpp>

#include <data/io/CsvLoader.h>
#include <data/io/JsonLoader.h>
#include <data/io/LoaderRegistry.h>
#include <data/io/NumpyLoader.h>

#include <QString>

#include <memory>

using namespace lumen::data::io;

TEST_CASE("LoaderRegistry -- singleton access", "[loader_registry]") {
    auto& reg1 = LoaderRegistry::instance();
    auto& reg2 = LoaderRegistry::instance();
    REQUIRE(&reg1 == &reg2);
}

TEST_CASE("LoaderRegistry -- register and find loader", "[loader_registry]") {
    auto& reg = LoaderRegistry::instance();
    reg.clear();

    reg.registerLoader(std::make_unique<CsvLoader>());

    auto* loader = reg.loaderForPath(QStringLiteral("/tmp/test.csv"));
    REQUIRE(loader != nullptr);
    REQUIRE(loader->supportedExtensions().contains(QStringLiteral("csv")));

    auto* noLoader = reg.loaderForPath(QStringLiteral("/tmp/test.xyz"));
    REQUIRE(noLoader == nullptr);

    reg.clear();
}

TEST_CASE("LoaderRegistry -- multiple loaders", "[loader_registry]") {
    auto& reg = LoaderRegistry::instance();
    reg.clear();

    reg.registerLoader(std::make_unique<CsvLoader>());
    reg.registerLoader(std::make_unique<JsonLoader>());
    reg.registerLoader(std::make_unique<NumpyLoader>());

    REQUIRE(reg.loaderCount() == 3);

    REQUIRE(reg.loaderForPath(QStringLiteral("data.csv")) != nullptr);
    REQUIRE(reg.loaderForPath(QStringLiteral("data.json")) != nullptr);
    REQUIRE(reg.loaderForPath(QStringLiteral("data.npy")) != nullptr);
    REQUIRE(reg.loaderForPath(QStringLiteral("data.parquet")) == nullptr);

    reg.clear();
}

TEST_CASE("LoaderRegistry -- allSupportedExtensions", "[loader_registry]") {
    auto& reg = LoaderRegistry::instance();
    reg.clear();

    reg.registerLoader(std::make_unique<CsvLoader>());
    reg.registerLoader(std::make_unique<JsonLoader>());

    QStringList exts = reg.allSupportedExtensions();
    REQUIRE(exts.contains(QStringLiteral("csv")));
    REQUIRE(exts.contains(QStringLiteral("tsv")));
    REQUIRE(exts.contains(QStringLiteral("json")));

    reg.clear();
}

TEST_CASE("LoaderRegistry -- fileFilter format", "[loader_registry]") {
    auto& reg = LoaderRegistry::instance();
    reg.clear();

    reg.registerLoader(std::make_unique<CsvLoader>());
    reg.registerLoader(std::make_unique<JsonLoader>());

    QString filter = reg.fileFilter();
    REQUIRE(filter.contains(QStringLiteral("*.csv")));
    REQUIRE(filter.contains(QStringLiteral("*.json")));
    REQUIRE(filter.contains(QStringLiteral(";;")));
    REQUIRE(filter.contains(QStringLiteral("All Supported")));

    reg.clear();
}

TEST_CASE("LoaderRegistry -- clear removes all loaders", "[loader_registry]") {
    auto& reg = LoaderRegistry::instance();
    reg.clear();

    reg.registerLoader(std::make_unique<CsvLoader>());
    REQUIRE(reg.loaderCount() == 1);

    reg.clear();
    REQUIRE(reg.loaderCount() == 0);
    REQUIRE(reg.loaderForPath(QStringLiteral("test.csv")) == nullptr);
}

TEST_CASE("LoaderRegistry -- null loader ignored", "[loader_registry]") {
    auto& reg = LoaderRegistry::instance();
    reg.clear();

    reg.registerLoader(nullptr);
    REQUIRE(reg.loaderCount() == 0);

    reg.clear();
}

TEST_CASE("LoaderRegistry -- case-insensitive extension matching", "[loader_registry]") {
    auto& reg = LoaderRegistry::instance();
    reg.clear();

    reg.registerLoader(std::make_unique<CsvLoader>());

    REQUIRE(reg.loaderForPath(QStringLiteral("test.CSV")) != nullptr);
    REQUIRE(reg.loaderForPath(QStringLiteral("test.Csv")) != nullptr);

    reg.clear();
}
