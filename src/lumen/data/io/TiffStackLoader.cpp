#ifdef LUMEN_HAS_TIFF

#include "TiffStackLoader.h"

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Unit.h>
#include <data/Volume3D.h>

#include <tiffio.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace lumen::data::io {

namespace {

/// RAII wrapper for TIFF*.
struct TiffFile {
    TIFF* tif = nullptr;
    explicit TiffFile(TIFF* t)
        : tif(t)
    {
    }
    ~TiffFile()
    {
        if (tif) {
            TIFFClose(tif);
        }
    }
    TiffFile(const TiffFile&) = delete;
    TiffFile& operator=(const TiffFile&) = delete;
};

} // namespace

QStringList TiffStackLoader::supportedExtensions() const
{
    return {QStringLiteral("tiff"), QStringLiteral("tif")};
}

std::shared_ptr<TabularBundle> TiffStackLoader::loadTabular(const QString& /*path*/)
{
    // TIFF data is image-based, not tabular
    return nullptr;
}

std::shared_ptr<Dataset> TiffStackLoader::loadDataset(const QString& path)
{
    std::string pathStr = path.toStdString();

    // Suppress libtiff warnings/errors during open
    TIFFSetWarningHandler(nullptr);
    TIFFSetErrorHandler(nullptr);

    TIFF* rawTif = TIFFOpen(pathStr.c_str(), "r");
    if (!rawTif) {
        throw std::runtime_error("TiffStackLoader: cannot open file: " + pathStr);
    }
    TiffFile tifFile(rawTif);

    // Read first page dimensions
    uint32_t width = 0;
    uint32_t height = 0;
    TIFFGetField(rawTif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(rawTif, TIFFTAG_IMAGELENGTH, &height);

    if (width == 0 || height == 0) {
        throw std::runtime_error("TiffStackLoader: invalid image dimensions");
    }

    // Count pages and read all data
    std::vector<std::vector<double>> pages;

    do {
        uint32_t pw = 0;
        uint32_t ph = 0;
        TIFFGetField(rawTif, TIFFTAG_IMAGEWIDTH, &pw);
        TIFFGetField(rawTif, TIFFTAG_IMAGELENGTH, &ph);

        if (pw != width || ph != height) {
            break; // All pages must have same dimensions
        }

        // Read via RGBA to handle any pixel format
        std::vector<uint32_t> raster(static_cast<std::size_t>(width) * height);
        if (!TIFFReadRGBAImageOriented(rawTif, width, height, raster.data(),
                                       ORIENTATION_TOPLEFT, 0)) {
            throw std::runtime_error("TiffStackLoader: cannot read TIFF page");
        }

        // Convert RGBA to double (grayscale luminance)
        std::vector<double> pageData(static_cast<std::size_t>(width) * height);
        for (std::size_t i = 0; i < pageData.size(); ++i) {
            uint32_t pixel = raster[i];
            auto r = static_cast<double>(TIFFGetR(pixel));
            auto g = static_cast<double>(TIFFGetG(pixel));
            auto b = static_cast<double>(TIFFGetB(pixel));
            // Standard luminance conversion
            pageData[i] = 0.2126 * r + 0.7152 * g + 0.0722 * b;
        }
        pages.push_back(std::move(pageData));
    } while (TIFFReadDirectory(rawTif));

    if (pages.empty()) {
        throw std::runtime_error("TiffStackLoader: no pages read from file");
    }

    Unit unit = Unit::dimensionless();
    std::size_t nx = static_cast<std::size_t>(width);
    std::size_t ny = static_cast<std::size_t>(height);

    if (pages.size() == 1) {
        // Single page -> Grid2D
        Dimension dimX{QStringLiteral("x"), unit, nx, CoordinateArray(0.0, 1.0, nx)};
        Dimension dimY{QStringLiteral("y"), unit, ny, CoordinateArray(0.0, 1.0, ny)};
        return std::make_shared<Grid2D>(QStringLiteral("tiff"), unit, std::move(dimX),
                                        std::move(dimY), std::move(pages[0]));
    }

    // Multi-page -> Volume3D
    std::size_t nz = pages.size();
    std::vector<double> volumeData;
    volumeData.reserve(nz * ny * nx);
    for (auto& page : pages) {
        volumeData.insert(volumeData.end(), page.begin(), page.end());
    }

    Dimension dimX{QStringLiteral("x"), unit, nx, CoordinateArray(0.0, 1.0, nx)};
    Dimension dimY{QStringLiteral("y"), unit, ny, CoordinateArray(0.0, 1.0, ny)};
    Dimension dimZ{QStringLiteral("z"), unit, nz, CoordinateArray(0.0, 1.0, nz)};
    return std::make_shared<Volume3D>(QStringLiteral("tiff_stack"), unit, std::move(dimX),
                                      std::move(dimY), std::move(dimZ), std::move(volumeData));
}

} // namespace lumen::data::io

#endif // LUMEN_HAS_TIFF
