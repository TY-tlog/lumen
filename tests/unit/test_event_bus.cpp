// Unit tests for EventBus.

#include <catch2/catch_test_macros.hpp>

#include <core/EventBus.h>

#include <QCoreApplication>
#include <QString>
#include <QVariant>

using namespace lumen::core;

// Helper: ensure QCoreApplication exists for event loop processing.
static QCoreApplication* ensureApp()
{
    if (QCoreApplication::instance() == nullptr) {
        static int argc = 1;
        static const char* argv[] = {"test"};
        static auto* app = new QCoreApplication(argc, const_cast<char**>(argv));
        return app;
    }
    return QCoreApplication::instance();
}

TEST_CASE("EventBus delivers event to subscriber", "[eventbus]") {
    ensureApp();

    EventBus bus;
    bool received = false;
    QString payload;

    bus.subscribe(Event::DocumentOpened, [&](const QVariant& v) {
        received = true;
        payload = v.toString();
    });

    bus.publish(Event::DocumentOpened, QVariant(QStringLiteral("test.csv")));

    // Process queued events.
    QCoreApplication::processEvents();

    REQUIRE(received);
    REQUIRE(payload == QStringLiteral("test.csv"));
}

TEST_CASE("EventBus delivers to multiple subscribers", "[eventbus]") {
    ensureApp();

    EventBus bus;
    int count = 0;

    bus.subscribe(Event::DocumentOpened, [&](const QVariant&) { ++count; });
    bus.subscribe(Event::DocumentOpened, [&](const QVariant&) { ++count; });

    bus.publish(Event::DocumentOpened);
    QCoreApplication::processEvents();

    REQUIRE(count == 2);
}

TEST_CASE("EventBus does not deliver wrong event type", "[eventbus]") {
    ensureApp();

    EventBus bus;
    bool received = false;

    bus.subscribe(Event::DocumentClosed, [&](const QVariant&) { received = true; });

    bus.publish(Event::DocumentOpened, QVariant(QStringLiteral("test.csv")));
    QCoreApplication::processEvents();

    REQUIRE_FALSE(received);
}

TEST_CASE("EventBus removes subscriptions when receiver is destroyed", "[eventbus]") {
    ensureApp();

    EventBus bus;
    int count = 0;

    auto* receiver = new QObject();
    bus.subscribe(Event::DocumentOpened, receiver, [&](const QVariant&) { ++count; });

    bus.publish(Event::DocumentOpened);
    QCoreApplication::processEvents();
    REQUIRE(count == 1);

    // Destroy the receiver — subscription should be removed.
    delete receiver;

    bus.publish(Event::DocumentOpened);
    QCoreApplication::processEvents();
    REQUIRE(count == 1);  // Should not have increased.
}

TEST_CASE("EventBus handles empty payload", "[eventbus]") {
    ensureApp();

    EventBus bus;
    bool received = false;

    bus.subscribe(Event::ThemeChanged, [&](const QVariant& v) {
        received = true;
        REQUIRE_FALSE(v.isValid());
    });

    bus.publish(Event::ThemeChanged);
    QCoreApplication::processEvents();

    REQUIRE(received);
}
