#ifdef LUMEN_HAS_HDF5

#include "Hdf5Loader.h"

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <data/Volume3D.h>

// HDF5 headers use C-style casts in macros (H5S_ALL, H5P_DEFAULT, etc.)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <hdf5.h>
#pragma GCC diagnostic pop

#include <stdexcept>
#include <string>
#include <vector>

// Wrap HDF5 macros that use C-style casts to avoid -Wold-style-cast
namespace {
const hid_t kH5S_ALL = static_cast<hid_t>(0);
const hid_t kH5P_DEFAULT = static_cast<hid_t>(0);
const hid_t kH5E_DEFAULT = static_cast<hid_t>(0);
} // namespace

namespace lumen::data::io {

namespace {

/// RAII wrapper for HDF5 identifiers.
struct H5Handle {
    hid_t id = -1;
    enum Kind { File, Dataset, Dataspace, Datatype };
    Kind kind;

    H5Handle(hid_t h, Kind k)
        : id(h)
        , kind(k)
    {
    }
    ~H5Handle()
    {
        if (id >= 0) {
            switch (kind) {
            case File:
                H5Fclose(id);
                break;
            case Dataset:
                H5Dclose(id);
                break;
            case Dataspace:
                H5Sclose(id);
                break;
            case Datatype:
                H5Tclose(id);
                break;
            }
        }
    }
    H5Handle(const H5Handle&) = delete;
    H5Handle& operator=(const H5Handle&) = delete;
};

/// Callback to find the first dataset name in an HDF5 group.
struct FirstDatasetFinder {
    std::string name;
    bool found = false;
};

herr_t findFirstDataset(hid_t /*loc_id*/, const char* name, const H5L_info_t* /*info*/,
                        void* opdata)
{
    auto* finder = static_cast<FirstDatasetFinder*>(opdata);
    finder->name = name;
    finder->found = true;
    return 1; // Stop iteration
}

} // namespace

QStringList Hdf5Loader::supportedExtensions() const
{
    return {QStringLiteral("h5"), QStringLiteral("hdf5"), QStringLiteral("hdf")};
}

std::shared_ptr<TabularBundle> Hdf5Loader::loadTabular(const QString& path)
{
    auto ds = loadDataset(path);
    if (!ds || ds->rank() != 1) {
        return nullptr;
    }
    // Wrap rank-1 dataset in a TabularBundle
    auto bundle = std::make_shared<TabularBundle>();
    auto r1 = std::dynamic_pointer_cast<Rank1Dataset>(ds);
    if (r1) {
        bundle->addColumn(std::move(r1));
    }
    return bundle;
}

// HDF5 macros (H5T_NATIVE_DOUBLE etc.) internally use C-style casts.
// Suppress -Wold-style-cast for the entire function body.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

std::shared_ptr<Dataset> Hdf5Loader::loadDataset(const QString& path)
{
    std::string pathStr = path.toStdString();

    // Suppress HDF5 default error output
    H5Eset_auto2(kH5E_DEFAULT, nullptr, nullptr);

    hid_t fileId = H5Fopen(pathStr.c_str(), H5F_ACC_RDONLY, kH5P_DEFAULT);
    if (fileId < 0) {
        throw std::runtime_error("Hdf5Loader: cannot open file: " + pathStr);
    }
    H5Handle fileHandle(fileId, H5Handle::File);

    // Find the first dataset in the root group
    FirstDatasetFinder finder;
    H5Literate(fileId, H5_INDEX_NAME, H5_ITER_INC, nullptr, findFirstDataset, &finder);
    if (!finder.found) {
        throw std::runtime_error("Hdf5Loader: no datasets found in file: " + pathStr);
    }

    hid_t dsId = H5Dopen2(fileId, finder.name.c_str(), kH5P_DEFAULT);
    if (dsId < 0) {
        throw std::runtime_error("Hdf5Loader: cannot open dataset: " + finder.name);
    }
    H5Handle dsHandle(dsId, H5Handle::Dataset);

    hid_t spaceId = H5Dget_space(dsId);
    if (spaceId < 0) {
        throw std::runtime_error("Hdf5Loader: cannot get dataspace");
    }
    H5Handle spaceHandle(spaceId, H5Handle::Dataspace);

    int ndims = H5Sget_simple_extent_ndims(spaceId);
    if (ndims < 1 || ndims > 3) {
        throw std::runtime_error("Hdf5Loader: unsupported rank " + std::to_string(ndims));
    }

    std::vector<hsize_t> dims(static_cast<std::size_t>(ndims));
    H5Sget_simple_extent_dims(spaceId, dims.data(), nullptr);

    // Read all data as doubles
    std::size_t totalElements = 1;
    for (int i = 0; i < ndims; ++i) {
        totalElements *= static_cast<std::size_t>(dims[static_cast<std::size_t>(i)]);
    }

    std::vector<double> data(totalElements);
    herr_t status = H5Dread(dsId, H5T_NATIVE_DOUBLE, kH5S_ALL, kH5S_ALL, kH5P_DEFAULT, data.data());
    if (status < 0) {
        throw std::runtime_error("Hdf5Loader: H5Dread failed");
    }

    QString dsName = QString::fromStdString(finder.name);
    Unit unit = Unit::dimensionless();

    if (ndims == 1) {
        auto r1 = std::make_shared<Rank1Dataset>(dsName, unit, std::move(data));
        return r1;
    }

    if (ndims == 2) {
        std::size_t nx = static_cast<std::size_t>(dims[1]); // columns
        std::size_t ny = static_cast<std::size_t>(dims[0]); // rows
        Dimension dimX{QStringLiteral("x"), unit, nx, CoordinateArray(0.0, 1.0, nx)};
        Dimension dimY{QStringLiteral("y"), unit, ny, CoordinateArray(0.0, 1.0, ny)};
        return std::make_shared<Grid2D>(dsName, unit, std::move(dimX), std::move(dimY),
                                        std::move(data));
    }

    // ndims == 3
    std::size_t nx = static_cast<std::size_t>(dims[2]);
    std::size_t ny = static_cast<std::size_t>(dims[1]);
    std::size_t nz = static_cast<std::size_t>(dims[0]);
    Dimension dimX{QStringLiteral("x"), unit, nx, CoordinateArray(0.0, 1.0, nx)};
    Dimension dimY{QStringLiteral("y"), unit, ny, CoordinateArray(0.0, 1.0, ny)};
    Dimension dimZ{QStringLiteral("z"), unit, nz, CoordinateArray(0.0, 1.0, nz)};
    return std::make_shared<Volume3D>(dsName, unit, std::move(dimX), std::move(dimY),
                                      std::move(dimZ), std::move(data));
}

#pragma GCC diagnostic pop

} // namespace lumen::data::io

#endif // LUMEN_HAS_HDF5
