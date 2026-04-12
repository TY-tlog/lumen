// Unit tests for ReactiveBinding.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <core/reactive/DependencyGraph.h>
#include <core/reactive/ReactiveBinding.h>
#include <core/reactive/ReactiveMode.h>
#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/MemoryManager.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <plot/PlotItem.h>

#include <QSignalSpy>

using namespace lumen::reactive;
using namespace lumen::data;
using Catch::Matchers::WithinRel;

namespace {

/// Minimal PlotItem for testing.
class StubPlotItem : public lumen::plot::PlotItem {
public:
    Type type() const override { return Type::Line; }
    QRectF dataBounds() const override { return {}; }
    void paint(QPainter*, const lumen::plot::CoordinateMapper&,
               const QRectF&) const override {}
    bool isVisible() const override { return true; }
    QString name() const override { return QStringLiteral("stub"); }
    QColor primaryColor() const override { return Qt::black; }
    void invalidate() override { invalidateCount++; }

    int invalidateCount = 0;
};

std::unique_ptr<Grid2D> makeGrid(std::vector<double> data)
{
    Dimension dimX{
        QStringLiteral("x"), Unit::dimensionless(), 3,
        CoordinateArray(0.0, 1.0, 3),
    };
    Dimension dimY{
        QStringLiteral("y"), Unit::dimensionless(), 2,
        CoordinateArray(0.0, 1.0, 2),
    };
    return std::make_unique<Grid2D>(
        QStringLiteral("test"), Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(data));
}

std::unique_ptr<Rank1Dataset> makeR1(std::vector<double> data)
{
    return std::make_unique<Rank1Dataset>(
        QStringLiteral("r1"), Unit::dimensionless(), std::move(data));
}

} // anonymous namespace

TEST_CASE("ReactiveBinding -- Static mode: snapshot isolation", "[reactive_binding]") {
    MemoryManager::instance().reset();

    StubPlotItem plotItem;
    auto grid = makeGrid({1.0, 2.0, 3.0, 4.0, 5.0, 6.0});

    ReactiveBinding binding(&plotItem, Mode::Static);
    binding.bindDataset(grid.get());

    // dataSource should return the snapshot, not the live dataset.
    auto* src = binding.dataSource();
    REQUIRE(src != nullptr);
    REQUIRE(src != grid.get());

    // Snapshot should have the same values.
    REQUIRE_THAT(src->valueAt({0, 0}), WithinRel(1.0));
    REQUIRE_THAT(src->valueAt({2, 1}), WithinRel(6.0));

    MemoryManager::instance().reset();
}

TEST_CASE("ReactiveBinding -- Static mode: snapshot not affected by live mutation", "[reactive_binding]") {
    MemoryManager::instance().reset();

    StubPlotItem plotItem;
    // Use Rank1Dataset since Grid2D doesn't expose mutable data.
    auto live = makeR1({10.0, 20.0, 30.0});

    ReactiveBinding binding(&plotItem, Mode::Static);
    binding.bindDataset(live.get());

    auto* snapshot = binding.dataSource();
    REQUIRE(snapshot != nullptr);
    REQUIRE_THAT(snapshot->valueAt({0}), WithinRel(10.0));

    // The snapshot is a clone, so we can't mutate through it, but we can verify
    // that dataSource returns the snapshot (not the live dataset).
    REQUIRE(snapshot != live.get());
    REQUIRE(snapshot->sizeBytes() == live->sizeBytes());

    MemoryManager::instance().reset();
}

TEST_CASE("ReactiveBinding -- DAG mode: changed() triggers repaintRequested", "[reactive_binding]") {
    StubPlotItem plotItem;
    auto ds = makeR1({1.0, 2.0});

    ReactiveBinding binding(&plotItem, Mode::DAG);
    binding.bindDataset(ds.get());

    QSignalSpy spy(&binding, &ReactiveBinding::repaintRequested);
    REQUIRE(spy.isValid());

    // dataSource should return the live dataset.
    REQUIRE(binding.dataSource() == ds.get());

    // Emit changed() on the dataset.
    emit ds->changed();

    REQUIRE(spy.count() == 1);
    REQUIRE(plotItem.invalidateCount == 1);
}

TEST_CASE("ReactiveBinding -- DAG mode: no snapshot exists", "[reactive_binding]") {
    StubPlotItem plotItem;
    auto ds = makeR1({1.0});

    ReactiveBinding binding(&plotItem, Mode::DAG);
    binding.bindDataset(ds.get());

    // In DAG mode, dataSource returns the live dataset directly.
    REQUIRE(binding.dataSource() == ds.get());
}

TEST_CASE("ReactiveBinding -- Bidirectional: generation counter prevents feedback", "[reactive_binding]") {
    StubPlotItem plotItem;
    auto ds = makeR1({1.0, 2.0});

    DependencyGraph graph;
    graph.addNode(ds.get());

    ReactiveBinding binding(&plotItem, Mode::Bidirectional);
    binding.setGraph(&graph);
    binding.bindDataset(ds.get());

    QSignalSpy spy(&binding, &ReactiveBinding::repaintRequested);

    // First change propagates normally.
    graph.propagate(ds.get());
    emit ds->changed();
    REQUIRE(spy.count() == 1);

    // Same generation: change should be suppressed.
    emit ds->changed();
    REQUIRE(spy.count() == 1); // Still 1 -- suppressed.

    // New propagation increments generation.
    graph.propagate(ds.get());
    emit ds->changed();
    REQUIRE(spy.count() == 2); // Now 2 -- new generation allowed it.
}

TEST_CASE("ReactiveBinding -- mode switching preserves data", "[reactive_binding]") {
    MemoryManager::instance().reset();

    StubPlotItem plotItem;
    auto ds = makeR1({5.0, 10.0, 15.0});

    ReactiveBinding binding(&plotItem, Mode::Static);
    binding.bindDataset(ds.get());

    // In Static mode, snapshot exists.
    auto* staticSrc = binding.dataSource();
    REQUIRE(staticSrc != ds.get());
    REQUIRE_THAT(staticSrc->valueAt({0}), WithinRel(5.0));

    // Switch to DAG: snapshot destroyed, live dataset returned.
    binding.setMode(Mode::DAG);
    REQUIRE(binding.dataSource() == ds.get());

    // Switch back to Static: new snapshot created.
    binding.setMode(Mode::Static);
    auto* newSnapshot = binding.dataSource();
    REQUIRE(newSnapshot != ds.get());
    REQUIRE_THAT(newSnapshot->valueAt({1}), WithinRel(10.0));

    MemoryManager::instance().reset();
}

TEST_CASE("ReactiveBinding -- MemoryManager tracks snapshot allocation", "[reactive_binding]") {
    MemoryManager::instance().reset();
    REQUIRE(MemoryManager::instance().currentUsageBytes() == 0);

    StubPlotItem plotItem;
    auto ds = makeR1({1.0, 2.0, 3.0}); // 3 * 8 = 24 bytes

    {
        ReactiveBinding binding(&plotItem, Mode::Static);
        binding.bindDataset(ds.get());

        // Snapshot allocated -- MemoryManager should have tracked it.
        REQUIRE(MemoryManager::instance().currentUsageBytes() == 3 * sizeof(double));
    }

    // Binding destroyed -- snapshot deallocated.
    REQUIRE(MemoryManager::instance().currentUsageBytes() == 0);

    MemoryManager::instance().reset();
}

TEST_CASE("ReactiveBinding -- MemoryManager tracks snapshot deallocation on mode switch", "[reactive_binding]") {
    MemoryManager::instance().reset();

    StubPlotItem plotItem;
    auto ds = makeR1({1.0, 2.0, 3.0, 4.0}); // 4 * 8 = 32 bytes

    ReactiveBinding binding(&plotItem, Mode::Static);
    binding.bindDataset(ds.get());
    REQUIRE(MemoryManager::instance().currentUsageBytes() == 4 * sizeof(double));

    // Switch to DAG: snapshot destroyed.
    binding.setMode(Mode::DAG);
    REQUIRE(MemoryManager::instance().currentUsageBytes() == 0);

    MemoryManager::instance().reset();
}

TEST_CASE("ReactiveBinding -- invalidate in Static mode re-snapshots", "[reactive_binding]") {
    MemoryManager::instance().reset();

    StubPlotItem plotItem;
    auto ds = makeR1({100.0, 200.0});

    ReactiveBinding binding(&plotItem, Mode::Static);
    binding.bindDataset(ds.get());

    auto* snap1 = binding.dataSource();
    REQUIRE(snap1 != nullptr);

    QSignalSpy spy(&binding, &ReactiveBinding::repaintRequested);

    // Invalidate re-snapshots.
    binding.invalidate();
    REQUIRE(spy.count() == 1);

    // The new snapshot may be a different pointer (re-cloned).
    auto* snap2 = binding.dataSource();
    REQUIRE(snap2 != nullptr);
    REQUIRE_THAT(snap2->valueAt({0}), WithinRel(100.0));

    MemoryManager::instance().reset();
}

TEST_CASE("ReactiveBinding -- modeChanged signal emitted", "[reactive_binding]") {
    StubPlotItem plotItem;
    ReactiveBinding binding(&plotItem, Mode::Static);

    QSignalSpy spy(&binding, &ReactiveBinding::modeChanged);

    binding.setMode(Mode::DAG);
    REQUIRE(spy.count() == 1);

    binding.setMode(Mode::Bidirectional);
    REQUIRE(spy.count() == 2);
}

TEST_CASE("ReactiveBinding -- setMode to same mode is a no-op", "[reactive_binding]") {
    StubPlotItem plotItem;
    ReactiveBinding binding(&plotItem, Mode::Static);

    QSignalSpy spy(&binding, &ReactiveBinding::modeChanged);
    binding.setMode(Mode::Static);
    REQUIRE(spy.count() == 0);
}
