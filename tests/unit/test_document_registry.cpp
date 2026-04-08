// Unit tests for DocumentRegistry.

#include <catch2/catch_test_macros.hpp>

#include <core/DocumentRegistry.h>
#include <core/EventBus.h>
#include <data/Column.h>
#include <data/DataFrame.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QString>

#include <memory>
#include <vector>

using namespace lumen::core;
using namespace lumen::data;

// Helper: ensure QCoreApplication exists for signal/slot processing.
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

/// Build a small test DataFrame wrapped in shared_ptr.
static std::shared_ptr<DataFrame> makeTestDf()
{
    std::vector<Column> cols;
    cols.emplace_back(QStringLiteral("x"), std::vector<int64_t>{1, 2, 3});
    return std::make_shared<DataFrame>(std::move(cols));
}

TEST_CASE("DocumentRegistry add and retrieve", "[docreg]") {
    ensureApp();

    DocumentRegistry reg;
    auto df = makeTestDf();

    const DataFrame* ptr = reg.addDocument(QStringLiteral("/tmp/a.csv"), df);
    REQUIRE(ptr != nullptr);
    REQUIRE(ptr->rowCount() == 3);
    REQUIRE(reg.count() == 1);

    // Retrieve by path.
    const DataFrame* found = reg.document(QStringLiteral("/tmp/a.csv"));
    REQUIRE(found == ptr);
}

TEST_CASE("DocumentRegistry returns nullptr for unknown path", "[docreg]") {
    ensureApp();

    DocumentRegistry reg;
    REQUIRE(reg.document(QStringLiteral("/no/such/file.csv")) == nullptr);
}

TEST_CASE("DocumentRegistry duplicate add returns existing", "[docreg]") {
    ensureApp();

    DocumentRegistry reg;
    auto df1 = makeTestDf();
    auto df2 = makeTestDf();

    const DataFrame* ptr1 = reg.addDocument(QStringLiteral("/tmp/dup.csv"), df1);
    const DataFrame* ptr2 = reg.addDocument(QStringLiteral("/tmp/dup.csv"), df2);

    REQUIRE(ptr1 == ptr2);
    REQUIRE(reg.count() == 1);
}

TEST_CASE("DocumentRegistry close removes document", "[docreg]") {
    ensureApp();

    DocumentRegistry reg;
    reg.addDocument(QStringLiteral("/tmp/close.csv"), makeTestDf());

    REQUIRE(reg.count() == 1);
    REQUIRE(reg.closeDocument(QStringLiteral("/tmp/close.csv")));
    REQUIRE(reg.count() == 0);
    REQUIRE(reg.document(QStringLiteral("/tmp/close.csv")) == nullptr);
}

TEST_CASE("DocumentRegistry close nonexistent returns false", "[docreg]") {
    ensureApp();

    DocumentRegistry reg;
    REQUIRE_FALSE(reg.closeDocument(QStringLiteral("/no/such.csv")));
}

TEST_CASE("DocumentRegistry emits documentOpened signal", "[docreg]") {
    ensureApp();

    DocumentRegistry reg;
    QSignalSpy spy(&reg, &DocumentRegistry::documentOpened);

    reg.addDocument(QStringLiteral("/tmp/sig.csv"), makeTestDf());

    REQUIRE(spy.count() == 1);
    REQUIRE(spy.at(0).at(0).toString() == QStringLiteral("/tmp/sig.csv"));
}

TEST_CASE("DocumentRegistry emits documentClosed signal", "[docreg]") {
    ensureApp();

    DocumentRegistry reg;
    reg.addDocument(QStringLiteral("/tmp/sig2.csv"), makeTestDf());

    QSignalSpy spy(&reg, &DocumentRegistry::documentClosed);
    reg.closeDocument(QStringLiteral("/tmp/sig2.csv"));

    REQUIRE(spy.count() == 1);
    REQUIRE(spy.at(0).at(0).toString() == QStringLiteral("/tmp/sig2.csv"));
}

TEST_CASE("DocumentRegistry publishes to EventBus", "[docreg]") {
    ensureApp();

    EventBus bus;
    DocumentRegistry reg(&bus);

    bool openReceived = false;
    bool closeReceived = false;

    bus.subscribe(Event::DocumentOpened, [&](const QVariant& v) {
        openReceived = true;
        REQUIRE(v.toString() == QStringLiteral("/tmp/bus.csv"));
    });

    bus.subscribe(Event::DocumentClosed, [&](const QVariant& v) {
        closeReceived = true;
        REQUIRE(v.toString() == QStringLiteral("/tmp/bus.csv"));
    });

    reg.addDocument(QStringLiteral("/tmp/bus.csv"), makeTestDf());
    QCoreApplication::processEvents();
    REQUIRE(openReceived);

    reg.closeDocument(QStringLiteral("/tmp/bus.csv"));
    QCoreApplication::processEvents();
    REQUIRE(closeReceived);
}
