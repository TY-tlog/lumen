#ifdef LUMEN_HAS_NETCDF

#include "NetCDFLoader.h"

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <data/Volume3D.h>

#include <netcdf.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace lumen::data::io {

namespace {

/// RAII wrapper for a NetCDF file handle.
struct NcFile {
    int ncid = -1;
    explicit NcFile(int id)
        : ncid(id)
    {
    }
    ~NcFile()
    {
        if (ncid >= 0) {
            nc_close(ncid);
        }
    }
    NcFile(const NcFile&) = delete;
    NcFile& operator=(const NcFile&) = delete;
};

void checkNc(int status, const char* msg)
{
    if (status != NC_NOERR) {
        throw std::runtime_error(std::string(msg) + ": " + nc_strerror(status));
    }
}

} // namespace

QStringList NetCDFLoader::supportedExtensions() const
{
    return {QStringLiteral("nc"), QStringLiteral("nc4"), QStringLiteral("netcdf")};
}

std::shared_ptr<TabularBundle> NetCDFLoader::loadTabular(const QString& path)
{
    auto ds = loadDataset(path);
    if (!ds || ds->rank() != 1) {
        return nullptr;
    }
    auto bundle = std::make_shared<TabularBundle>();
    auto r1 = std::dynamic_pointer_cast<Rank1Dataset>(ds);
    if (r1) {
        bundle->addColumn(std::move(r1));
    }
    return bundle;
}

std::shared_ptr<Dataset> NetCDFLoader::loadDataset(const QString& path)
{
    std::string pathStr = path.toStdString();

    int ncid = -1;
    checkNc(nc_open(pathStr.c_str(), NC_NOWRITE, &ncid), "NetCDFLoader: cannot open file");
    NcFile file(ncid);

    // Find the number of variables
    int nvars = 0;
    checkNc(nc_inq_nvars(ncid, &nvars), "NetCDFLoader: cannot query variables");
    if (nvars == 0) {
        throw std::runtime_error("NetCDFLoader: no variables found in file");
    }

    // Find the first numeric variable (float or double)
    int targetVarId = -1;
    char varName[NC_MAX_NAME + 1];
    nc_type varType = 0;
    int varNdims = 0;

    for (int vid = 0; vid < nvars; ++vid) {
        nc_type vtype = 0;
        int vndims = 0;
        checkNc(nc_inq_var(ncid, vid, varName, &vtype, &vndims, nullptr, nullptr),
                "NetCDFLoader: cannot query variable");
        if ((vtype == NC_FLOAT || vtype == NC_DOUBLE || vtype == NC_INT || vtype == NC_SHORT)
            && vndims >= 1 && vndims <= 3) {
            targetVarId = vid;
            varType = vtype;
            varNdims = vndims;
            break;
        }
    }

    if (targetVarId < 0) {
        throw std::runtime_error("NetCDFLoader: no suitable numeric variable found");
    }

    // Get dimension sizes
    std::vector<int> dimIds(static_cast<std::size_t>(varNdims));
    checkNc(nc_inq_var(ncid, targetVarId, nullptr, nullptr, nullptr, dimIds.data(), nullptr),
            "NetCDFLoader: cannot get dimension ids");

    std::vector<std::size_t> dimSizes(static_cast<std::size_t>(varNdims));
    for (int d = 0; d < varNdims; ++d) {
        std::size_t len = 0;
        checkNc(nc_inq_dimlen(ncid, dimIds[static_cast<std::size_t>(d)], &len),
                "NetCDFLoader: cannot get dimension length");
        dimSizes[static_cast<std::size_t>(d)] = len;
    }

    // Read data as doubles
    std::size_t totalElements = 1;
    for (auto s : dimSizes) {
        totalElements *= s;
    }

    std::vector<double> data(totalElements);
    checkNc(nc_get_var_double(ncid, targetVarId, data.data()),
            "NetCDFLoader: cannot read variable data");

    QString dsName = QString::fromUtf8(varName);
    Unit unit = Unit::dimensionless();

    (void)varType; // already read as double

    if (varNdims == 1) {
        return std::make_shared<Rank1Dataset>(dsName, unit, std::move(data));
    }

    if (varNdims == 2) {
        std::size_t ny = dimSizes[0]; // first dim = rows
        std::size_t nx = dimSizes[1]; // second dim = columns
        Dimension dimX{QStringLiteral("x"), unit, nx, CoordinateArray(0.0, 1.0, nx)};
        Dimension dimY{QStringLiteral("y"), unit, ny, CoordinateArray(0.0, 1.0, ny)};
        return std::make_shared<Grid2D>(dsName, unit, std::move(dimX), std::move(dimY),
                                        std::move(data));
    }

    // varNdims == 3
    std::size_t nz = dimSizes[0];
    std::size_t ny = dimSizes[1];
    std::size_t nx = dimSizes[2];
    Dimension dimX{QStringLiteral("x"), unit, nx, CoordinateArray(0.0, 1.0, nx)};
    Dimension dimY{QStringLiteral("y"), unit, ny, CoordinateArray(0.0, 1.0, ny)};
    Dimension dimZ{QStringLiteral("z"), unit, nz, CoordinateArray(0.0, 1.0, nz)};
    return std::make_shared<Volume3D>(dsName, unit, std::move(dimX), std::move(dimY),
                                      std::move(dimZ), std::move(data));
}

} // namespace lumen::data::io

#endif // LUMEN_HAS_NETCDF
