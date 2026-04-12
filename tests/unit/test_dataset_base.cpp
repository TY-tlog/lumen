// Unit tests for Dataset abstract base class using a mock subclass.

#include <catch2/catch_test_macros.hpp>

#include <data/CoordinateArray.h>
#include <data/Dataset.h>
#include <data/Dimension.h>
#include <data/Unit.h>

#include <QSignalSpy>

using namespace lumen::data;

namespace {

/// Minimal concrete Dataset for testing the abstract base.
class MockDataset : public Dataset {
    Q_OBJECT
public:
    explicit MockDataset(QObject* parent = nullptr)
        : Dataset(parent)
    {
    }

    [[nodiscard]] QString name() const override { return QStringLiteral("mock"); }

    [[nodiscard]] std::vector<Dimension> dimensions() const override
    {
        return {Dimension{
            QStringLiteral("x"),
            Unit::dimensionless(),
            3,
            CoordinateArray(0.0, 1.0, 3),
        }};
    }

    [[nodiscard]] Unit valueUnit() const override { return Unit::dimensionless(); }
    [[nodiscard]] StorageMode storageMode() const override { return StorageMode::InMemory; }
    [[nodiscard]] std::size_t rank() const override { return 1; }
    [[nodiscard]] std::vector<std::size_t> shape() const override { return {3}; }
    [[nodiscard]] std::size_t sizeBytes() const override { return 3 * sizeof(double); }

    [[nodiscard]] double valueAt(const std::vector<std::size_t>& index) const override
    {
        return static_cast<double>(index[0]) * 10.0;
    }

    [[nodiscard]] std::unique_ptr<Dataset> clone() const override
    {
        return std::make_unique<MockDataset>();
    }

    void triggerChanged() { emit changed(); }
    void triggerCoordinatesChanged() { emit coordinatesChanged(); }
};

} // anonymous namespace

TEST_CASE("Dataset mock subclass rank and shape", "[dataset]") {
    MockDataset ds;
    REQUIRE(ds.rank() == 1);
    REQUIRE(ds.shape().size() == 1);
    REQUIRE(ds.shape()[0] == 3);
    REQUIRE(ds.name() == QStringLiteral("mock"));
    REQUIRE(ds.storageMode() == Dataset::StorageMode::InMemory);
}

TEST_CASE("Dataset mock valueAt", "[dataset]") {
    MockDataset ds;
    REQUIRE(ds.valueAt({0}) == 0.0);
    REQUIRE(ds.valueAt({1}) == 10.0);
    REQUIRE(ds.valueAt({2}) == 20.0);
}

TEST_CASE("Dataset changed signal fires", "[dataset]") {
    MockDataset ds;
    QSignalSpy spy(&ds, &Dataset::changed);
    REQUIRE(spy.isValid());

    ds.triggerChanged();
    REQUIRE(spy.count() == 1);
}

TEST_CASE("Dataset coordinatesChanged signal fires", "[dataset]") {
    MockDataset ds;
    QSignalSpy spy(&ds, &Dataset::coordinatesChanged);
    REQUIRE(spy.isValid());

    ds.triggerCoordinatesChanged();
    REQUIRE(spy.count() == 1);
}

TEST_CASE("Dataset sizeBytes", "[dataset]") {
    MockDataset ds;
    REQUIRE(ds.sizeBytes() == 3 * sizeof(double));
}

#include "test_dataset_base.moc"
