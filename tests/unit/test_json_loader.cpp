// Unit tests for JsonLoader.

#include <catch2/catch_test_macros.hpp>

#include <data/io/JsonLoader.h>
#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QString>
#include <QTemporaryDir>

#include <cmath>
#include <limits>
#include <memory>

using namespace lumen::data;
using namespace lumen::data::io;

namespace {

QString writeTemp(QTemporaryDir& dir, const char* name, const QByteArray& content)
{
    QString path = dir.path() + QStringLiteral("/") + QString::fromUtf8(name);
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write(content);
    f.close();
    return path;
}

} // namespace

TEST_CASE("JsonLoader -- supported extensions", "[json_loader]") {
    JsonLoader loader;
    REQUIRE(loader.supportedExtensions().contains(QStringLiteral("json")));
}

TEST_CASE("JsonLoader -- canLoad recognizes .json", "[json_loader]") {
    JsonLoader loader;
    REQUIRE(loader.canLoad(QStringLiteral("/tmp/test.json")));
    REQUIRE_FALSE(loader.canLoad(QStringLiteral("/tmp/test.csv")));
}

TEST_CASE("JsonLoader -- array of objects", "[json_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    QByteArray json = R"([{"x": 1.0, "y": 2.0}, {"x": 3.0, "y": 4.0}, {"x": 5.0, "y": 6.0}])";
    QString path = writeTemp(dir, "test.json", json);

    JsonLoader loader;
    auto bundle = loader.loadTabular(path);

    REQUIRE(bundle != nullptr);
    REQUIRE(bundle->columnCount() == 2);
    REQUIRE(bundle->rowCount() == 3);
    REQUIRE(bundle->columnByName(QStringLiteral("x")) != nullptr);
    REQUIRE(bundle->columnByName(QStringLiteral("y")) != nullptr);
    REQUIRE(bundle->columnByName(QStringLiteral("x"))->doubleData()[0] == 1.0);
    REQUIRE(bundle->columnByName(QStringLiteral("y"))->doubleData()[2] == 6.0);
}

TEST_CASE("JsonLoader -- mixed types", "[json_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    QByteArray json = R"([{"name": "Alice", "score": 95}, {"name": "Bob", "score": 87}])";
    QString path = writeTemp(dir, "test.json", json);

    JsonLoader loader;
    auto bundle = loader.loadTabular(path);

    REQUIRE(bundle != nullptr);
    REQUIRE(bundle->columnCount() == 2);
    REQUIRE(bundle->rowCount() == 2);

    auto nameCol = bundle->columnByName(QStringLiteral("name"));
    REQUIRE(nameCol != nullptr);
    CHECK_NOTHROW(nameCol->stringData());
    REQUIRE(nameCol->stringData()[0] == QStringLiteral("Alice"));

    auto scoreCol = bundle->columnByName(QStringLiteral("score"));
    REQUIRE(scoreCol != nullptr);
    CHECK_NOTHROW(scoreCol->doubleData());
    REQUIRE(scoreCol->doubleData()[1] == 87.0);
}

TEST_CASE("JsonLoader -- null values become NaN", "[json_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    QByteArray json = R"([{"val": 1.0}, {"val": null}, {"val": 3.0}])";
    QString path = writeTemp(dir, "test.json", json);

    JsonLoader loader;
    auto bundle = loader.loadTabular(path);

    REQUIRE(bundle != nullptr);
    REQUIRE(bundle->columnCount() == 1);
    REQUIRE(bundle->rowCount() == 3);
    REQUIRE(bundle->column(0)->doubleData()[0] == 1.0);
    REQUIRE(std::isnan(bundle->column(0)->doubleData()[1]));
    REQUIRE(bundle->column(0)->doubleData()[2] == 3.0);
}

TEST_CASE("JsonLoader -- empty array", "[json_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    QByteArray json = R"([])";
    QString path = writeTemp(dir, "test.json", json);

    JsonLoader loader;
    auto bundle = loader.loadTabular(path);

    REQUIRE(bundle != nullptr);
    REQUIRE(bundle->columnCount() == 0);
}

TEST_CASE("JsonLoader -- non-array top level throws", "[json_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    QByteArray json = R"({"key": "value"})";
    QString path = writeTemp(dir, "test.json", json);

    JsonLoader loader;
    REQUIRE_THROWS(loader.loadTabular(path));
}
