#ifdef LUMEN_HAS_MATIO

#include "MatLoader.h"

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <data/Volume3D.h>

#include <matio.h>

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace lumen::data::io {

namespace {

/// RAII wrapper for mat_t*.
struct MatFile {
    mat_t* mat = nullptr;
    explicit MatFile(mat_t* m)
        : mat(m)
    {
    }
    ~MatFile()
    {
        if (mat) {
            Mat_Close(mat);
        }
    }
    MatFile(const MatFile&) = delete;
    MatFile& operator=(const MatFile&) = delete;
};

/// RAII wrapper for matvar_t*.
struct MatVar {
    matvar_t* var = nullptr;
    explicit MatVar(matvar_t* v)
        : var(v)
    {
    }
    ~MatVar()
    {
        if (var) {
            Mat_VarFree(var);
        }
    }
    MatVar(const MatVar&) = delete;
    MatVar& operator=(const MatVar&) = delete;
};

} // namespace

QStringList MatLoader::supportedExtensions() const
{
    return {QStringLiteral("mat")};
}

std::shared_ptr<TabularBundle> MatLoader::loadTabular(const QString& path)
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

std::shared_ptr<Dataset> MatLoader::loadDataset(const QString& path)
{
    std::string pathStr = path.toStdString();

    mat_t* rawMat = Mat_Open(pathStr.c_str(), MAT_ACC_RDONLY);
    if (!rawMat) {
        throw std::runtime_error("MatLoader: cannot open file: " + pathStr);
    }
    MatFile matFile(rawMat);

    // Find the first numeric variable
    matvar_t* rawVar = nullptr;
    while ((rawVar = Mat_VarReadNext(rawMat)) != nullptr) {
        if (rawVar->class_type == MAT_C_DOUBLE || rawVar->class_type == MAT_C_SINGLE
            || rawVar->class_type == MAT_C_INT32 || rawVar->class_type == MAT_C_INT64
            || rawVar->class_type == MAT_C_UINT32 || rawVar->class_type == MAT_C_INT16
            || rawVar->class_type == MAT_C_UINT16 || rawVar->class_type == MAT_C_INT8
            || rawVar->class_type == MAT_C_UINT8) {
            break;
        }
        Mat_VarFree(rawVar);
        rawVar = nullptr;
    }

    if (!rawVar) {
        throw std::runtime_error("MatLoader: no numeric variable found in file");
    }
    MatVar matVar(rawVar);

    if (rawVar->rank < 1 || rawVar->rank > 3) {
        throw std::runtime_error("MatLoader: unsupported rank "
                                 + std::to_string(rawVar->rank));
    }

    // Calculate total elements and determine effective rank
    // MATLAB stores column-major; dims[0]=rows, dims[1]=cols
    std::size_t totalElements = 1;
    int effectiveRank = rawVar->rank;
    std::vector<std::size_t> dims;
    for (int d = 0; d < rawVar->rank; ++d) {
        std::size_t s = rawVar->dims[d];
        dims.push_back(s);
        totalElements *= s;
    }

    // Handle the case where rank=2 but one dimension is 1 (effectively 1D)
    if (effectiveRank == 2 && (dims[0] == 1 || dims[1] == 1)) {
        effectiveRank = 1;
    }

    // Convert data to doubles
    std::vector<double> data(totalElements);

    if (rawVar->class_type == MAT_C_DOUBLE) {
        std::memcpy(data.data(), rawVar->data, totalElements * sizeof(double));
    } else if (rawVar->class_type == MAT_C_SINGLE) {
        const auto* src = static_cast<const float*>(rawVar->data);
        for (std::size_t i = 0; i < totalElements; ++i) {
            data[i] = static_cast<double>(src[i]);
        }
    } else if (rawVar->class_type == MAT_C_INT32) {
        const auto* src = static_cast<const int32_t*>(rawVar->data);
        for (std::size_t i = 0; i < totalElements; ++i) {
            data[i] = static_cast<double>(src[i]);
        }
    } else if (rawVar->class_type == MAT_C_INT64) {
        const auto* src = static_cast<const int64_t*>(rawVar->data);
        for (std::size_t i = 0; i < totalElements; ++i) {
            data[i] = static_cast<double>(src[i]);
        }
    } else if (rawVar->class_type == MAT_C_UINT32) {
        const auto* src = static_cast<const uint32_t*>(rawVar->data);
        for (std::size_t i = 0; i < totalElements; ++i) {
            data[i] = static_cast<double>(src[i]);
        }
    } else if (rawVar->class_type == MAT_C_INT16) {
        const auto* src = static_cast<const int16_t*>(rawVar->data);
        for (std::size_t i = 0; i < totalElements; ++i) {
            data[i] = static_cast<double>(src[i]);
        }
    } else if (rawVar->class_type == MAT_C_UINT16) {
        const auto* src = static_cast<const uint16_t*>(rawVar->data);
        for (std::size_t i = 0; i < totalElements; ++i) {
            data[i] = static_cast<double>(src[i]);
        }
    } else if (rawVar->class_type == MAT_C_INT8) {
        const auto* src = static_cast<const int8_t*>(rawVar->data);
        for (std::size_t i = 0; i < totalElements; ++i) {
            data[i] = static_cast<double>(src[i]);
        }
    } else if (rawVar->class_type == MAT_C_UINT8) {
        const auto* src = static_cast<const uint8_t*>(rawVar->data);
        for (std::size_t i = 0; i < totalElements; ++i) {
            data[i] = static_cast<double>(src[i]);
        }
    }

    QString dsName = QString::fromUtf8(rawVar->name);
    Unit unit = Unit::dimensionless();

    if (effectiveRank == 1) {
        return std::make_shared<Rank1Dataset>(dsName, unit, std::move(data));
    }

    if (effectiveRank == 2) {
        // MATLAB: dims[0]=rows, dims[1]=cols, stored column-major
        // We need row-major for Grid2D
        std::size_t ny = dims[0]; // rows
        std::size_t nx = dims[1]; // cols
        std::vector<double> rowMajor(totalElements);
        for (std::size_t row = 0; row < ny; ++row) {
            for (std::size_t col = 0; col < nx; ++col) {
                rowMajor[row * nx + col] = data[col * ny + row]; // column-major to row-major
            }
        }
        Dimension dimX{QStringLiteral("x"), unit, nx, CoordinateArray(0.0, 1.0, nx)};
        Dimension dimY{QStringLiteral("y"), unit, ny, CoordinateArray(0.0, 1.0, ny)};
        return std::make_shared<Grid2D>(dsName, unit, std::move(dimX), std::move(dimY),
                                        std::move(rowMajor));
    }

    // effectiveRank == 3
    // MATLAB: dims[0]=rows(Y), dims[1]=cols(X), dims[2]=depth(Z), column-major
    std::size_t ny = dims[0];
    std::size_t nx = dims[1];
    std::size_t nz = dims[2];
    std::vector<double> rowMajor(totalElements);
    for (std::size_t z = 0; z < nz; ++z) {
        for (std::size_t y = 0; y < ny; ++y) {
            for (std::size_t x = 0; x < nx; ++x) {
                std::size_t colMajorIdx = z * (ny * nx) + x * ny + y;
                std::size_t rowMajorIdx = z * (ny * nx) + y * nx + x;
                rowMajor[rowMajorIdx] = data[colMajorIdx];
            }
        }
    }
    Dimension dimX{QStringLiteral("x"), unit, nx, CoordinateArray(0.0, 1.0, nx)};
    Dimension dimY{QStringLiteral("y"), unit, ny, CoordinateArray(0.0, 1.0, ny)};
    Dimension dimZ{QStringLiteral("z"), unit, nz, CoordinateArray(0.0, 1.0, nz)};
    return std::make_shared<Volume3D>(dsName, unit, std::move(dimX), std::move(dimY),
                                      std::move(dimZ), std::move(rowMajor));
}

} // namespace lumen::data::io

#endif // LUMEN_HAS_MATIO
