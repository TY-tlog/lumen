#include "NumpyLoader.h"

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>

#include <QFile>
#include <QIODevice>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace lumen::data::io {

namespace {

/// Parse .npy header and return shape + element size in bytes + whether it's float.
struct NpyHeader {
    std::vector<std::size_t> shape;
    std::size_t elementBytes = 0;
    bool isFloat = false;
    bool isLittleEndian = true;
    std::size_t headerSize = 0; // Total header size (magic + version + HEADER_LEN + dict)
};

NpyHeader parseNpyHeader(const QByteArray& raw)
{
    // Magic: \x93NUMPY
    if (raw.size() < 10) {
        throw std::runtime_error("NumpyLoader: file too small for .npy header");
    }

    if (static_cast<unsigned char>(raw[0]) != 0x93 || raw[1] != 'N' || raw[2] != 'U'
        || raw[3] != 'M' || raw[4] != 'P' || raw[5] != 'Y') {
        throw std::runtime_error("NumpyLoader: invalid .npy magic number");
    }

    // Version
    // uint8_t major = static_cast<uint8_t>(raw[6]);
    uint8_t minor = static_cast<uint8_t>(raw[7]);

    // Header length
    std::size_t headerLen = 0;
    std::size_t headerStart = 0;
    if (minor < 2) {
        // Version 1.0: 2-byte little-endian HEADER_LEN
        uint16_t hl = 0;
        std::memcpy(&hl, raw.constData() + 8, 2);
        headerLen = hl;
        headerStart = 10;
    } else {
        // Version 2.0: 4-byte little-endian HEADER_LEN
        uint32_t hl = 0;
        std::memcpy(&hl, raw.constData() + 8, 4);
        headerLen = hl;
        headerStart = 12;
    }

    if (static_cast<std::size_t>(raw.size()) < headerStart + headerLen) {
        throw std::runtime_error("NumpyLoader: header truncated");
    }

    std::string headerDict(raw.constData() + headerStart, headerLen);

    NpyHeader hdr;
    hdr.headerSize = headerStart + headerLen;

    // Parse descr (dtype)
    auto descrPos = headerDict.find("'descr'");
    if (descrPos == std::string::npos) {
        descrPos = headerDict.find("\"descr\"");
    }
    if (descrPos != std::string::npos) {
        auto quoteStart = headerDict.find('\'', descrPos + 7);
        if (quoteStart == std::string::npos) {
            quoteStart = headerDict.find('"', descrPos + 7);
        }
        if (quoteStart != std::string::npos) {
            auto quoteEnd = headerDict.find(headerDict[quoteStart], quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                std::string descr = headerDict.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                // e.g. "<f8", ">f4", "<i4"
                if (!descr.empty()) {
                    hdr.isLittleEndian = (descr[0] == '<' || descr[0] == '|');
                    hdr.isFloat = (descr.find('f') != std::string::npos);
                    // Parse element size
                    std::string sizeStr;
                    for (char c : descr) {
                        if (c >= '0' && c <= '9') {
                            sizeStr += c;
                        }
                    }
                    if (!sizeStr.empty()) {
                        hdr.elementBytes = std::stoull(sizeStr);
                    }
                }
            }
        }
    }

    // Parse shape
    auto shapePos = headerDict.find("'shape'");
    if (shapePos == std::string::npos) {
        shapePos = headerDict.find("\"shape\"");
    }
    if (shapePos != std::string::npos) {
        auto parenStart = headerDict.find('(', shapePos);
        auto parenEnd = headerDict.find(')', parenStart);
        if (parenStart != std::string::npos && parenEnd != std::string::npos) {
            std::string shapeStr = headerDict.substr(parenStart + 1, parenEnd - parenStart - 1);
            // Parse comma-separated integers
            std::size_t pos = 0;
            while (pos < shapeStr.size()) {
                // Skip whitespace
                while (pos < shapeStr.size() && (shapeStr[pos] == ' ' || shapeStr[pos] == ',')) {
                    ++pos;
                }
                if (pos >= shapeStr.size()) {
                    break;
                }
                std::size_t numStart = pos;
                while (pos < shapeStr.size() && shapeStr[pos] >= '0' && shapeStr[pos] <= '9') {
                    ++pos;
                }
                if (pos > numStart) {
                    hdr.shape.push_back(std::stoull(shapeStr.substr(numStart, pos - numStart)));
                }
            }
        }
    }

    if (hdr.elementBytes == 0) {
        hdr.elementBytes = 8; // Default to float64
        hdr.isFloat = true;
    }

    return hdr;
}

} // namespace

QStringList NumpyLoader::supportedExtensions() const
{
    return {QStringLiteral("npy")};
}

std::shared_ptr<TabularBundle> NumpyLoader::loadTabular(const QString& path)
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

std::shared_ptr<Dataset> NumpyLoader::loadDataset(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("NumpyLoader: cannot open file: " + path.toStdString());
    }

    QByteArray raw = file.readAll();
    NpyHeader hdr = parseNpyHeader(raw);

    if (hdr.shape.empty()) {
        throw std::runtime_error("NumpyLoader: empty shape in .npy header");
    }

    std::size_t totalElements = 1;
    for (auto s : hdr.shape) {
        totalElements *= s;
    }

    std::size_t dataOffset = hdr.headerSize;
    std::size_t expectedBytes = totalElements * hdr.elementBytes;
    if (static_cast<std::size_t>(raw.size()) < dataOffset + expectedBytes) {
        throw std::runtime_error("NumpyLoader: file truncated");
    }

    const char* dataPtr = raw.constData() + dataOffset;

    // Convert to doubles
    std::vector<double> data(totalElements);

    if (hdr.isFloat && hdr.elementBytes == 8) {
        // float64
        std::memcpy(data.data(), dataPtr, totalElements * sizeof(double));
    } else if (hdr.isFloat && hdr.elementBytes == 4) {
        // float32
        for (std::size_t i = 0; i < totalElements; ++i) {
            float val = 0;
            std::memcpy(&val, dataPtr + i * 4, 4);
            data[i] = static_cast<double>(val);
        }
    } else if (!hdr.isFloat && hdr.elementBytes == 8) {
        // int64
        for (std::size_t i = 0; i < totalElements; ++i) {
            int64_t val = 0;
            std::memcpy(&val, dataPtr + i * 8, 8);
            data[i] = static_cast<double>(val);
        }
    } else if (!hdr.isFloat && hdr.elementBytes == 4) {
        // int32
        for (std::size_t i = 0; i < totalElements; ++i) {
            int32_t val = 0;
            std::memcpy(&val, dataPtr + i * 4, 4);
            data[i] = static_cast<double>(val);
        }
    } else {
        throw std::runtime_error("NumpyLoader: unsupported dtype (elementBytes="
                                 + std::to_string(hdr.elementBytes) + ", float="
                                 + (hdr.isFloat ? "true" : "false") + ")");
    }

    Unit unit = Unit::dimensionless();
    QString dsName = QStringLiteral("numpy_data");

    if (hdr.shape.size() == 1) {
        return std::make_shared<Rank1Dataset>(dsName, unit, std::move(data));
    }

    if (hdr.shape.size() == 2) {
        std::size_t ny = hdr.shape[0]; // rows
        std::size_t nx = hdr.shape[1]; // cols
        Dimension dimX{QStringLiteral("x"), unit, nx, CoordinateArray(0.0, 1.0, nx)};
        Dimension dimY{QStringLiteral("y"), unit, ny, CoordinateArray(0.0, 1.0, ny)};
        return std::make_shared<Grid2D>(dsName, unit, std::move(dimX), std::move(dimY),
                                        std::move(data));
    }

    // Higher ranks: treat as flattened 1D
    return std::make_shared<Rank1Dataset>(dsName, unit, std::move(data));
}

} // namespace lumen::data::io
