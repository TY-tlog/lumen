// Unit tests for DependencyGraph.

#include <catch2/catch_test_macros.hpp>

#include <core/reactive/DependencyGraph.h>
#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/Grid2D.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>

#include <QSignalSpy>

#include <algorithm>
#include <stdexcept>

using namespace lumen::reactive;
using namespace lumen::data;

namespace {

std::unique_ptr<Rank1Dataset> makeR1(const QString& name, std::vector<double> data)
{
    return std::make_unique<Rank1Dataset>(name, Unit::dimensionless(), std::move(data));
}

} // anonymous namespace

TEST_CASE("DependencyGraph -- addNode does not crash on nullptr", "[dependency_graph]") {
    DependencyGraph graph;
    graph.addNode(nullptr); // Should be a no-op.
}

TEST_CASE("DependencyGraph -- addNode and downstream empty", "[dependency_graph]") {
    DependencyGraph graph;
    auto ds = makeR1(QStringLiteral("a"), {1.0, 2.0});
    graph.addNode(ds.get());

    auto down = graph.downstream(ds.get());
    REQUIRE(down.empty());
}

TEST_CASE("DependencyGraph -- addDerivation creates edge", "[dependency_graph]") {
    DependencyGraph graph;
    auto a = makeR1(QStringLiteral("a"), {1.0});
    auto b = makeR1(QStringLiteral("b"), {2.0});

    bool updateCalled = false;
    graph.addDerivation(a.get(), b.get(), [&] { updateCalled = true; });

    auto down = graph.downstream(a.get());
    REQUIRE(down.size() == 1);
    REQUIRE(down[0] == b.get());

    // Propagate triggers the update function.
    graph.propagate(a.get());
    REQUIRE(updateCalled);
}

TEST_CASE("DependencyGraph -- propagate increments generation counter", "[dependency_graph]") {
    DependencyGraph graph;
    auto a = makeR1(QStringLiteral("a"), {1.0});
    graph.addNode(a.get());

    REQUIRE(graph.generation() == 0);
    graph.propagate(a.get());
    REQUIRE(graph.generation() == 1);
    graph.propagate(a.get());
    REQUIRE(graph.generation() == 2);
}

TEST_CASE("DependencyGraph -- cycle detection rejects self-loop", "[dependency_graph]") {
    DependencyGraph graph;
    auto a = makeR1(QStringLiteral("a"), {1.0});

    REQUIRE_THROWS_AS(
        graph.addDerivation(a.get(), a.get(), [] {}),
        std::invalid_argument);
}

TEST_CASE("DependencyGraph -- cycle detection rejects circular derivation", "[dependency_graph]") {
    DependencyGraph graph;
    auto a = makeR1(QStringLiteral("a"), {1.0});
    auto b = makeR1(QStringLiteral("b"), {2.0});
    auto c = makeR1(QStringLiteral("c"), {3.0});

    // a -> b -> c, then c -> a would be a cycle.
    graph.addDerivation(a.get(), b.get(), [] {});
    graph.addDerivation(b.get(), c.get(), [] {});

    REQUIRE_THROWS_AS(
        graph.addDerivation(c.get(), a.get(), [] {}),
        std::invalid_argument);
}

TEST_CASE("DependencyGraph -- downstream returns transitive dependents", "[dependency_graph]") {
    DependencyGraph graph;
    auto a = makeR1(QStringLiteral("a"), {1.0});
    auto b = makeR1(QStringLiteral("b"), {2.0});
    auto c = makeR1(QStringLiteral("c"), {3.0});

    graph.addDerivation(a.get(), b.get(), [] {});
    graph.addDerivation(b.get(), c.get(), [] {});

    auto down = graph.downstream(a.get());
    REQUIRE(down.size() == 2);

    // Both b and c should be in the result.
    REQUIRE(std::find(down.begin(), down.end(), b.get()) != down.end());
    REQUIRE(std::find(down.begin(), down.end(), c.get()) != down.end());
}

TEST_CASE("DependencyGraph -- propagate emits nodeUpdated for all affected", "[dependency_graph]") {
    DependencyGraph graph;
    auto a = makeR1(QStringLiteral("a"), {1.0});
    auto b = makeR1(QStringLiteral("b"), {2.0});
    auto c = makeR1(QStringLiteral("c"), {3.0});

    graph.addDerivation(a.get(), b.get(), [] {});
    graph.addDerivation(b.get(), c.get(), [] {});

    QSignalSpy spy(&graph, &DependencyGraph::nodeUpdated);
    graph.propagate(a.get());

    // Should emit for a, b, and c.
    REQUIRE(spy.count() == 3);
}

TEST_CASE("DependencyGraph -- propagate invokes update functions in order", "[dependency_graph]") {
    DependencyGraph graph;
    auto a = makeR1(QStringLiteral("a"), {1.0});
    auto b = makeR1(QStringLiteral("b"), {2.0});
    auto c = makeR1(QStringLiteral("c"), {3.0});

    std::vector<QString> order;
    graph.addDerivation(a.get(), b.get(), [&] { order.push_back(QStringLiteral("b")); });
    graph.addDerivation(b.get(), c.get(), [&] { order.push_back(QStringLiteral("c")); });

    graph.propagate(a.get());

    REQUIRE(order.size() == 2);
    REQUIRE(order[0] == QStringLiteral("b"));
    REQUIRE(order[1] == QStringLiteral("c"));
}

TEST_CASE("DependencyGraph -- addDerivation rejects null pointers", "[dependency_graph]") {
    DependencyGraph graph;
    auto a = makeR1(QStringLiteral("a"), {1.0});

    REQUIRE_THROWS_AS(graph.addDerivation(nullptr, a.get(), [] {}), std::invalid_argument);
    REQUIRE_THROWS_AS(graph.addDerivation(a.get(), nullptr, [] {}), std::invalid_argument);
}

TEST_CASE("DependencyGraph -- diamond dependency does not duplicate updates", "[dependency_graph]") {
    DependencyGraph graph;
    auto a = makeR1(QStringLiteral("a"), {1.0});
    auto b = makeR1(QStringLiteral("b"), {2.0});
    auto c = makeR1(QStringLiteral("c"), {3.0});
    auto d = makeR1(QStringLiteral("d"), {4.0});

    // Diamond: a -> b -> d, a -> c -> d
    graph.addDerivation(a.get(), b.get(), [] {});
    graph.addDerivation(a.get(), c.get(), [] {});
    graph.addDerivation(b.get(), d.get(), [] {});
    graph.addDerivation(c.get(), d.get(), [] {});

    QSignalSpy spy(&graph, &DependencyGraph::nodeUpdated);
    graph.propagate(a.get());

    // a, b, c, d -- each once.
    REQUIRE(spy.count() == 4);
}
