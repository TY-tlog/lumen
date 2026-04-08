// PlotRegistry unit tests.

#include <catch2/catch_test_macros.hpp>

#include <core/EventBus.h>
#include <core/PlotRegistry.h>

#include <QSignalSpy>
#include <QObject>

using namespace lumen::core;

TEST_CASE("PlotRegistry register and lookup", "[plotregistry]") {
    EventBus bus;
    PlotRegistry registry(&bus);

    QObject canvas;
    registry.registerPlot("/path/to/file.csv", &canvas);

    REQUIRE(registry.plotFor("/path/to/file.csv") == &canvas);
    REQUIRE(registry.count() == 1);
}

TEST_CASE("PlotRegistry lookup returns nullptr for unknown path", "[plotregistry]") {
    PlotRegistry registry;
    REQUIRE(registry.plotFor("/nonexistent") == nullptr);
}

TEST_CASE("PlotRegistry unregister removes entry", "[plotregistry]") {
    PlotRegistry registry;
    QObject canvas;

    registry.registerPlot("/path", &canvas);
    REQUIRE(registry.count() == 1);

    registry.unregisterPlot("/path");
    REQUIRE(registry.count() == 0);
    REQUIRE(registry.plotFor("/path") == nullptr);
}

TEST_CASE("PlotRegistry destroyed canvas auto-cleans", "[plotregistry]") {
    PlotRegistry registry;

    auto* canvas = new QObject();
    registry.registerPlot("/path", canvas);
    REQUIRE(registry.count() == 1);

    delete canvas;
    REQUIRE(registry.count() == 0);
    REQUIRE(registry.plotFor("/path") == nullptr);
}

TEST_CASE("PlotRegistry clear removes all entries", "[plotregistry]") {
    PlotRegistry registry;
    QObject canvas1;
    QObject canvas2;

    registry.registerPlot("/a", &canvas1);
    registry.registerPlot("/b", &canvas2);
    REQUIRE(registry.count() == 2);

    registry.clear();
    REQUIRE(registry.count() == 0);
}

TEST_CASE("PlotRegistry emits signals on register/unregister", "[plotregistry]") {
    PlotRegistry registry;
    QObject canvas;

    QSignalSpy regSpy(&registry, &PlotRegistry::plotRegistered);
    QSignalSpy unregSpy(&registry, &PlotRegistry::plotUnregistered);

    registry.registerPlot("/path", &canvas);
    REQUIRE(regSpy.count() == 1);
    REQUIRE(regSpy.first().first().toString() == "/path");

    registry.unregisterPlot("/path");
    REQUIRE(unregSpy.count() == 1);
    REQUIRE(unregSpy.first().first().toString() == "/path");
}

TEST_CASE("PlotRegistry publishes EventBus PlotCreated on register", "[plotregistry]") {
    EventBus bus;
    PlotRegistry registry(&bus);
    QObject canvas;

    bool received = false;
    QString receivedPath;
    bus.subscribe(Event::PlotCreated, [&](const QVariant& payload) {
        received = true;
        receivedPath = payload.toString();
    });

    registry.registerPlot("/test.csv", &canvas);

    // EventBus uses queued connections — process pending events.
    QCoreApplication::processEvents();

    REQUIRE(received);
    REQUIRE(receivedPath == "/test.csv");
}
