// Unit tests for NetCDFLoader (creates NetCDF fixtures programmatically).

#ifdef LUMEN_HAS_NETCDF

#include <catch2/catch_test_macros.hpp>

#include <data/io/NetCDFLoader.h>
#include <data/Grid2D.h>
#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>

#include <netcdf.h>

#include <QTemporaryDir>

#include <cstddef>
#include <string>
#include <vector>

using namespace lumen::data;
using namespace lumen::data::io;

namespace {

void checkNc(int status, const char* msg)
{
    if (status != NC_NOERR) {
        throw std::runtime_error(std::string(msg) + ": " + nc_strerror(status));
    }
}

/// Create a temporary NetCDF file with a 1D double variable.
QString createNetcdf_1D(QTemporaryDir& dir, const std::vector<double>& values)
{
    QString path = dir.path() + QStringLiteral("/test1d.nc");
    std::string pathStr = path.toStdString();

    int ncid = 0;
    checkNc(nc_create(pathStr.c_str(), NC_CLOBBER, &ncid), "create");

    int dimId = 0;
    checkNc(nc_def_dim(ncid, "x", values.size(), &dimId), "def_dim");

    int varId = 0;
    checkNc(nc_def_var(ncid, "data1d", NC_DOUBLE, 1, &dimId, &varId), "def_var");

    checkNc(nc_enddef(ncid), "enddef");
    checkNc(nc_put_var_double(ncid, varId, values.data()), "put_var");
    checkNc(nc_close(ncid), "close");

    return path;
}

/// Create a temporary NetCDF file with a 2D double variable.
QString createNetcdf_2D(QTemporaryDir& dir, std::size_t rows, std::size_t cols,
                        const std::vector<double>& values)
{
    QString path = dir.path() + QStringLiteral("/test2d.nc");
    std::string pathStr = path.toStdString();

    int ncid = 0;
    checkNc(nc_create(pathStr.c_str(), NC_CLOBBER, &ncid), "create");

    int dimIds[2] = {};
    checkNc(nc_def_dim(ncid, "y", rows, &dimIds[0]), "def_dim_y");
    checkNc(nc_def_dim(ncid, "x", cols, &dimIds[1]), "def_dim_x");

    int varId = 0;
    checkNc(nc_def_var(ncid, "grid2d", NC_DOUBLE, 2, dimIds, &varId), "def_var");

    checkNc(nc_enddef(ncid), "enddef");
    checkNc(nc_put_var_double(ncid, varId, values.data()), "put_var");
    checkNc(nc_close(ncid), "close");

    return path;
}

} // namespace

TEST_CASE("NetCDFLoader -- supported extensions", "[netcdf_loader]") {
    NetCDFLoader loader;
    QStringList exts = loader.supportedExtensions();
    REQUIRE(exts.contains(QStringLiteral("nc")));
    REQUIRE(exts.contains(QStringLiteral("nc4")));
}

TEST_CASE("NetCDFLoader -- load 1D dataset", "[netcdf_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    std::vector<double> values = {10.0, 20.0, 30.0, 40.0};
    QString path = createNetcdf_1D(dir, values);

    NetCDFLoader loader;
    auto ds = loader.loadDataset(path);

    REQUIRE(ds != nullptr);
    REQUIRE(ds->rank() == 1);
    REQUIRE(ds->shape()[0] == 4);
    REQUIRE(ds->valueAt({0}) == 10.0);
    REQUIRE(ds->valueAt({3}) == 40.0);
}

TEST_CASE("NetCDFLoader -- load 1D as TabularBundle", "[netcdf_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    std::vector<double> values = {5.0, 6.0, 7.0};
    QString path = createNetcdf_1D(dir, values);

    NetCDFLoader loader;
    auto bundle = loader.loadTabular(path);

    REQUIRE(bundle != nullptr);
    REQUIRE(bundle->columnCount() == 1);
    REQUIRE(bundle->rowCount() == 3);
    REQUIRE(bundle->column(0)->doubleData()[0] == 5.0);
}

TEST_CASE("NetCDFLoader -- load 2D dataset as Grid2D", "[netcdf_loader]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    // 3 rows x 4 cols
    std::vector<double> values = {
        1.0, 2.0, 3.0, 4.0,
        5.0, 6.0, 7.0, 8.0,
        9.0, 10.0, 11.0, 12.0};
    QString path = createNetcdf_2D(dir, 3, 4, values);

    NetCDFLoader loader;
    auto ds = loader.loadDataset(path);

    REQUIRE(ds != nullptr);
    REQUIRE(ds->rank() == 2);
    auto shape = ds->shape();
    REQUIRE(shape[0] == 4); // nx
    REQUIRE(shape[1] == 3); // ny
}

TEST_CASE("NetCDFLoader -- nonexistent file throws", "[netcdf_loader]") {
    NetCDFLoader loader;
    REQUIRE_THROWS(loader.loadDataset(QStringLiteral("/nonexistent/path.nc")));
}

#else

// Placeholder test so the file compiles without NetCDF
#include <catch2/catch_test_macros.hpp>

TEST_CASE("NetCDFLoader -- skipped (NetCDF not available)", "[netcdf_loader]") {
    SUCCEED("NetCDF not available, tests skipped");
}

#endif // LUMEN_HAS_NETCDF
