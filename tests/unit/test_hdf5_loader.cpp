// Unit tests for Hdf5Loader (creates HDF5 fixtures programmatically).

#ifdef LUMEN_HAS_HDF5

#include <catch2/catch_test_macros.hpp>

#include <data/io/Hdf5Loader.h>
#include <data/Grid2D.h>
#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>
#include <data/Volume3D.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <hdf5.h>
#pragma GCC diagnostic pop

#include <QTemporaryDir>

#include <cstddef>
#include <string>
#include <vector>

using namespace lumen::data;
using namespace lumen::data::io;

// HDF5 macros use C-style casts internally; suppress for test helpers.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

namespace {

/// Create a temporary HDF5 file with a 1D dataset.
QString createHdf5_1D(QTemporaryDir& dir, const std::vector<double>& values)
{
    QString path = dir.path() + QStringLiteral("/test1d.h5");
    std::string pathStr = path.toStdString();

    hid_t file = H5Fcreate(pathStr.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    hsize_t dims[1] = {values.size()};
    hid_t space = H5Screate_simple(1, dims, nullptr);
    hid_t dset = H5Dcreate2(file, "data1d", H5T_NATIVE_DOUBLE, space,
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, values.data());
    H5Dclose(dset);
    H5Sclose(space);
    H5Fclose(file);

    return path;
}

/// Create a temporary HDF5 file with a 2D dataset.
QString createHdf5_2D(QTemporaryDir& dir, std::size_t rows, std::size_t cols,
                      const std::vector<double>& values)
{
    QString path = dir.path() + QStringLiteral("/test2d.h5");
    std::string pathStr = path.toStdString();

    hid_t file = H5Fcreate(pathStr.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    hsize_t dims[2] = {static_cast<hsize_t>(rows), static_cast<hsize_t>(cols)};
    hid_t space = H5Screate_simple(2, dims, nullptr);
    hid_t dset = H5Dcreate2(file, "grid2d", H5T_NATIVE_DOUBLE, space,
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, values.data());
    H5Dclose(dset);
    H5Sclose(space);
    H5Fclose(file);

    return path;
}

/// Create a temporary HDF5 file with a 3D dataset.
QString createHdf5_3D(QTemporaryDir& dir, std::size_t nz, std::size_t ny, std::size_t nx,
                      const std::vector<double>& values)
{
    QString path = dir.path() + QStringLiteral("/test3d.h5");
    std::string pathStr = path.toStdString();

    hid_t file = H5Fcreate(pathStr.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    hsize_t dims[3] = {static_cast<hsize_t>(nz), static_cast<hsize_t>(ny),
                       static_cast<hsize_t>(nx)};
    hid_t space = H5Screate_simple(3, dims, nullptr);
    hid_t dset = H5Dcreate2(file, "volume3d", H5T_NATIVE_DOUBLE, space,
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, values.data());
    H5Dclose(dset);
    H5Sclose(space);
    H5Fclose(file);

    return path;
}

} // namespace

#pragma GCC diagnostic pop

TEST_CASE("Hdf5Loader -- supported extensions", "[hdf5_loader]") {
    Hdf5Loader loader;
    QStringList exts = loader.supportedExtensions();
    REQUIRE(exts.contains(QStringLiteral("h5")));
    REQUIRE(exts.contains(QStringLiteral("hdf5")));
    REQUIRE(exts.contains(QStringLiteral("hdf")));
}

TEST_CASE("Hdf5Loader -- load 1D dataset", "[hdf5_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    std::vector<double> values = {1.0, 2.0, 3.0, 4.0, 5.0};
    QString path = createHdf5_1D(dir, values);

    Hdf5Loader loader;
    auto ds = loader.loadDataset(path);

    REQUIRE(ds != nullptr);
    REQUIRE(ds->rank() == 1);
    REQUIRE(ds->shape()[0] == 5);
    REQUIRE(ds->valueAt({0}) == 1.0);
    REQUIRE(ds->valueAt({4}) == 5.0);
}

TEST_CASE("Hdf5Loader -- load 1D as TabularBundle", "[hdf5_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    std::vector<double> values = {10.0, 20.0, 30.0};
    QString path = createHdf5_1D(dir, values);

    Hdf5Loader loader;
    auto bundle = loader.loadTabular(path);

    REQUIRE(bundle != nullptr);
    REQUIRE(bundle->columnCount() == 1);
    REQUIRE(bundle->rowCount() == 3);
    REQUIRE(bundle->column(0)->doubleData()[1] == 20.0);
}

TEST_CASE("Hdf5Loader -- load 2D dataset as Grid2D", "[hdf5_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    // 3 rows x 4 cols
    std::vector<double> values = {
        1.0, 2.0, 3.0, 4.0,
        5.0, 6.0, 7.0, 8.0,
        9.0, 10.0, 11.0, 12.0};
    QString path = createHdf5_2D(dir, 3, 4, values);

    Hdf5Loader loader;
    auto ds = loader.loadDataset(path);

    REQUIRE(ds != nullptr);
    REQUIRE(ds->rank() == 2);
    auto shape = ds->shape();
    REQUIRE(shape[0] == 4); // nx = cols
    REQUIRE(shape[1] == 3); // ny = rows
}

TEST_CASE("Hdf5Loader -- load 3D dataset as Volume3D", "[hdf5_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    // 2 x 3 x 4 volume
    std::vector<double> values(2 * 3 * 4);
    for (std::size_t i = 0; i < values.size(); ++i) {
        values[i] = static_cast<double>(i);
    }
    QString path = createHdf5_3D(dir, 2, 3, 4, values);

    Hdf5Loader loader;
    auto ds = loader.loadDataset(path);

    REQUIRE(ds != nullptr);
    REQUIRE(ds->rank() == 3);
    auto shape = ds->shape();
    REQUIRE(shape[0] == 4); // nx
    REQUIRE(shape[1] == 3); // ny
    REQUIRE(shape[2] == 2); // nz
}

TEST_CASE("Hdf5Loader -- nonexistent file throws", "[hdf5_loader]") {
    Hdf5Loader loader;
    REQUIRE_THROWS(loader.loadDataset(QStringLiteral("/nonexistent/path.h5")));
}

#else

// Placeholder test so the file compiles without HDF5
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Hdf5Loader -- skipped (HDF5 not available)", "[hdf5_loader]") {
    SUCCEED("HDF5 not available, tests skipped");
}

#endif // LUMEN_HAS_HDF5
