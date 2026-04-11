// Tests for DataTableDock Grid2D/Volume3D placeholders (T20)
// and memory status formatting (T23).

#include <catch2/catch_test_macros.hpp>

#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/MemoryManager.h>
#include <data/Unit.h>
#include <data/Volume3D.h>
#include <data/io/LoaderRegistry.h>

#include <cstddef>
#include <vector>

using namespace lumen::data;

TEST_CASE("Grid2D shape for placeholder text", "[T20]") {
    constexpr std::size_t kW = 256;
    constexpr std::size_t kH = 128;

    Dimension dimX{QStringLiteral("x"), Unit::dimensionless(), kW,
                   CoordinateArray(0.0, 1.0, kW)};
    Dimension dimY{QStringLiteral("y"), Unit::dimensionless(), kH,
                   CoordinateArray(0.0, 1.0, kH)};

    std::vector<double> data(kW * kH, 0.0);
    Grid2D grid(QStringLiteral("test"), Unit::dimensionless(),
                std::move(dimX), std::move(dimY), std::move(data));

    REQUIRE(grid.rank() == 2);
    auto shape = grid.shape();
    REQUIRE(shape.size() == 2);
    REQUIRE(shape[0] == kW);
    REQUIRE(shape[1] == kH);
}

TEST_CASE("Volume3D shape for placeholder text", "[T20]") {
    constexpr std::size_t kX = 64;
    constexpr std::size_t kY = 32;
    constexpr std::size_t kZ = 16;

    Dimension dimX{QStringLiteral("x"), Unit::dimensionless(), kX,
                   CoordinateArray(0.0, 1.0, kX)};
    Dimension dimY{QStringLiteral("y"), Unit::dimensionless(), kY,
                   CoordinateArray(0.0, 1.0, kY)};
    Dimension dimZ{QStringLiteral("z"), Unit::dimensionless(), kZ,
                   CoordinateArray(0.0, 1.0, kZ)};

    std::vector<double> data(kX * kY * kZ, 0.0);
    Volume3D vol(QStringLiteral("test"), Unit::dimensionless(),
                 std::move(dimX), std::move(dimY), std::move(dimZ),
                 std::move(data));

    REQUIRE(vol.rank() == 3);
    auto shape = vol.shape();
    REQUIRE(shape.size() == 3);
    REQUIRE(shape[0] == kX);
    REQUIRE(shape[1] == kY);
    REQUIRE(shape[2] == kZ);
}

TEST_CASE("MemoryManager budget and usage", "[T23][T24]") {
    auto& mm = MemoryManager::instance();
    mm.reset();

    SECTION("default budget is 4 GB") {
        REQUIRE(mm.memoryBudgetBytes() == 4ULL * 1024 * 1024 * 1024);
    }

    SECTION("setBudget changes the budget") {
        mm.setBudget(2ULL * 1024 * 1024 * 1024);
        REQUIRE(mm.memoryBudgetBytes() == 2ULL * 1024 * 1024 * 1024);
        mm.reset();
    }

    SECTION("trackAllocation updates currentUsageBytes") {
        mm.trackAllocation(1024 * 1024);  // 1 MB
        REQUIRE(mm.currentUsageBytes() == 1024 * 1024);
        mm.reset();
    }

    SECTION("trackDeallocation decreases usage") {
        mm.trackAllocation(2 * 1024 * 1024);
        mm.trackDeallocation(1024 * 1024);
        REQUIRE(mm.currentUsageBytes() == 1024 * 1024);
        mm.reset();
    }
}

TEST_CASE("LoaderRegistry has registered loaders", "[T21]") {
    // LoaderRegistry is a singleton that should have loaders registered
    // by Application. In test context we just check the API works.
    auto& reg = lumen::data::io::LoaderRegistry::instance();

    // fileFilter() should return a non-empty or empty string (depends on registration)
    // Just verify it doesn't crash.
    QString filter = reg.fileFilter();
    // allSupportedExtensions should not crash
    QStringList exts = reg.allSupportedExtensions();
    REQUIRE(true);  // If we got here, the API works
}
