// Unit tests for NumpyLoader (creates .npy binary fixtures in-test).

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <data/io/NumpyLoader.h>
#include <data/Grid2D.h>
#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QString>
#include <QTemporaryDir>

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

using namespace lumen::data;
using namespace lumen::data::io;

namespace {

/// Write a minimal .npy file (version 1.0, little-endian float64).
QString writeNpy1D(QTemporaryDir& dir, const char* name, const std::vector<double>& values)
{
    // Build header dict
    std::string headerDict = "{'descr': '<f8', 'fortran_order': False, 'shape': ("
                             + std::to_string(values.size()) + ",), }";
    // Pad to align data to 64 bytes
    std::size_t headerDictLen = headerDict.size();
    std::size_t totalHeaderLen = 10 + headerDictLen; // magic(6) + version(2) + headerLen(2) + dict
    std::size_t padding = (64 - (totalHeaderLen % 64)) % 64;
    headerDict.append(padding, ' ');
    headerDict.back() = '\n';
    headerDictLen = headerDict.size();

    QByteArray data;
    // Magic
    data.append(static_cast<char>(0x93));
    data.append("NUMPY", 5);
    // Version 1.0
    data.append(static_cast<char>(1));
    data.append(static_cast<char>(0));
    // Header length (2 bytes, little-endian)
    uint16_t hl = static_cast<uint16_t>(headerDictLen);
    data.append(reinterpret_cast<const char*>(&hl), 2);
    // Header dict
    data.append(headerDict.c_str(), static_cast<int>(headerDictLen));
    // Data
    data.append(reinterpret_cast<const char*>(values.data()),
                static_cast<int>(values.size() * sizeof(double)));

    QString path = dir.path() + QStringLiteral("/") + QString::fromUtf8(name);
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write(data);
    f.close();
    return path;
}

/// Write a 2D .npy file (version 1.0, little-endian float64, C order).
QString writeNpy2D(QTemporaryDir& dir, const char* name, std::size_t rows, std::size_t cols,
                   const std::vector<double>& values)
{
    std::string headerDict = "{'descr': '<f8', 'fortran_order': False, 'shape': ("
                             + std::to_string(rows) + ", " + std::to_string(cols) + "), }";
    std::size_t totalHeaderLen = 10 + headerDict.size();
    std::size_t padding = (64 - (totalHeaderLen % 64)) % 64;
    headerDict.append(padding, ' ');
    headerDict.back() = '\n';

    QByteArray data;
    data.append(static_cast<char>(0x93));
    data.append("NUMPY", 5);
    data.append(static_cast<char>(1));
    data.append(static_cast<char>(0));
    uint16_t hl = static_cast<uint16_t>(headerDict.size());
    data.append(reinterpret_cast<const char*>(&hl), 2);
    data.append(headerDict.c_str(), static_cast<int>(headerDict.size()));
    data.append(reinterpret_cast<const char*>(values.data()),
                static_cast<int>(values.size() * sizeof(double)));

    QString path = dir.path() + QStringLiteral("/") + QString::fromUtf8(name);
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write(data);
    f.close();
    return path;
}

/// Write a 1D float32 .npy file.
QString writeNpyFloat32(QTemporaryDir& dir, const char* name, const std::vector<float>& values)
{
    std::string headerDict = "{'descr': '<f4', 'fortran_order': False, 'shape': ("
                             + std::to_string(values.size()) + ",), }";
    std::size_t totalHeaderLen = 10 + headerDict.size();
    std::size_t padding = (64 - (totalHeaderLen % 64)) % 64;
    headerDict.append(padding, ' ');
    headerDict.back() = '\n';

    QByteArray data;
    data.append(static_cast<char>(0x93));
    data.append("NUMPY", 5);
    data.append(static_cast<char>(1));
    data.append(static_cast<char>(0));
    uint16_t hl = static_cast<uint16_t>(headerDict.size());
    data.append(reinterpret_cast<const char*>(&hl), 2);
    data.append(headerDict.c_str(), static_cast<int>(headerDict.size()));
    data.append(reinterpret_cast<const char*>(values.data()),
                static_cast<int>(values.size() * sizeof(float)));

    QString path = dir.path() + QStringLiteral("/") + QString::fromUtf8(name);
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write(data);
    f.close();
    return path;
}

} // namespace

TEST_CASE("NumpyLoader -- supported extensions", "[numpy_loader]") {
    NumpyLoader loader;
    REQUIRE(loader.supportedExtensions().contains(QStringLiteral("npy")));
}

TEST_CASE("NumpyLoader -- canLoad", "[numpy_loader]") {
    NumpyLoader loader;
    REQUIRE(loader.canLoad(QStringLiteral("/tmp/test.npy")));
    REQUIRE_FALSE(loader.canLoad(QStringLiteral("/tmp/test.csv")));
}

TEST_CASE("NumpyLoader -- 1D float64", "[numpy_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    std::vector<double> values = {1.0, 2.0, 3.0, 4.0, 5.0};
    QString path = writeNpy1D(dir, "test1d.npy", values);

    NumpyLoader loader;
    auto ds = loader.loadDataset(path);

    REQUIRE(ds != nullptr);
    REQUIRE(ds->rank() == 1);
    REQUIRE(ds->shape()[0] == 5);
    REQUIRE(ds->valueAt({0}) == 1.0);
    REQUIRE(ds->valueAt({4}) == 5.0);
}

TEST_CASE("NumpyLoader -- 1D as TabularBundle", "[numpy_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    std::vector<double> values = {10.0, 20.0, 30.0};
    QString path = writeNpy1D(dir, "test1d_tab.npy", values);

    NumpyLoader loader;
    auto bundle = loader.loadTabular(path);

    REQUIRE(bundle != nullptr);
    REQUIRE(bundle->columnCount() == 1);
    REQUIRE(bundle->rowCount() == 3);
    REQUIRE(bundle->column(0)->doubleData()[0] == 10.0);
    REQUIRE(bundle->column(0)->doubleData()[2] == 30.0);
}

TEST_CASE("NumpyLoader -- 2D float64", "[numpy_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    // 3x4 matrix (row-major)
    std::vector<double> values = {
        1.0, 2.0, 3.0, 4.0,
        5.0, 6.0, 7.0, 8.0,
        9.0, 10.0, 11.0, 12.0};
    QString path = writeNpy2D(dir, "test2d.npy", 3, 4, values);

    NumpyLoader loader;
    auto ds = loader.loadDataset(path);

    REQUIRE(ds != nullptr);
    REQUIRE(ds->rank() == 2);
    auto shape = ds->shape();
    REQUIRE(shape[0] == 4); // cols = nx
    REQUIRE(shape[1] == 3); // rows = ny
    // Grid2D: valueAt({col, row})
    REQUIRE(ds->valueAt({0, 0}) == 1.0);
    REQUIRE(ds->valueAt({3, 2}) == 12.0);
}

TEST_CASE("NumpyLoader -- 1D float32", "[numpy_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    std::vector<float> values = {1.5f, 2.5f, 3.5f};
    QString path = writeNpyFloat32(dir, "test_f32.npy", values);

    NumpyLoader loader;
    auto ds = loader.loadDataset(path);

    REQUIRE(ds != nullptr);
    REQUIRE(ds->rank() == 1);
    REQUIRE(ds->shape()[0] == 3);
    REQUIRE(ds->valueAt({0}) == Catch::Approx(1.5));
    REQUIRE(ds->valueAt({2}) == Catch::Approx(3.5));
}

TEST_CASE("NumpyLoader -- invalid magic throws", "[numpy_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    QString path = dir.path() + QStringLiteral("/bad.npy");
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write("this is not a numpy file");
    f.close();

    NumpyLoader loader;
    REQUIRE_THROWS(loader.loadDataset(path));
}
