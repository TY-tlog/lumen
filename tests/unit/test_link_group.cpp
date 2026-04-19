#include <catch2/catch_test_macros.hpp>

#include <dashboard/LinkGroup.h>

#include <QSignalSpy>

using namespace lumen::dashboard;

TEST_CASE("LinkGroup: default channels are X-axis and Crosshair", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("default"));
    REQUIRE(lg.isChannelEnabled(LinkChannel::XAxis));
    REQUIRE_FALSE(lg.isChannelEnabled(LinkChannel::YAxis));
    REQUIRE(lg.isChannelEnabled(LinkChannel::Crosshair));
    REQUIRE_FALSE(lg.isChannelEnabled(LinkChannel::Selection));
}

TEST_CASE("LinkGroup: name is stored", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("group-A"));
    REQUIRE(lg.name() == "group-A");
}

TEST_CASE("LinkGroup: addPanel / removePanel / hasPanel", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.addPanel(0);
    lg.addPanel(1);
    REQUIRE(lg.hasPanel(0));
    REQUIRE(lg.hasPanel(1));
    REQUIRE_FALSE(lg.hasPanel(2));

    lg.removePanel(0);
    REQUIRE_FALSE(lg.hasPanel(0));
    REQUIRE(lg.hasPanel(1));
}

TEST_CASE("LinkGroup: addPanel is idempotent", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.addPanel(0);
    lg.addPanel(0);
    REQUIRE(lg.panels().size() == 1);
}

TEST_CASE("LinkGroup: setChannelEnabled toggles channel", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.setChannelEnabled(LinkChannel::YAxis, true);
    REQUIRE(lg.isChannelEnabled(LinkChannel::YAxis));

    lg.setChannelEnabled(LinkChannel::XAxis, false);
    REQUIRE_FALSE(lg.isChannelEnabled(LinkChannel::XAxis));
}

TEST_CASE("LinkGroup: propagateXRange emits signal when enabled", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.addPanel(0);
    lg.addPanel(1);

    QSignalSpy spy(&lg, &LinkGroup::xRangeChanged);
    lg.propagateXRange(0, 1.0, 10.0);
    REQUIRE(spy.count() == 1);
    REQUIRE(spy.at(0).at(0).toInt() == 0);
    REQUIRE(spy.at(0).at(1).toDouble() == 1.0);
    REQUIRE(spy.at(0).at(2).toDouble() == 10.0);
}

TEST_CASE("LinkGroup: propagateXRange silent when channel disabled", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.addPanel(0);
    lg.setChannelEnabled(LinkChannel::XAxis, false);

    QSignalSpy spy(&lg, &LinkGroup::xRangeChanged);
    lg.propagateXRange(0, 1.0, 10.0);
    REQUIRE(spy.count() == 0);
}

TEST_CASE("LinkGroup: propagateYRange emits when enabled", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.addPanel(0);
    lg.setChannelEnabled(LinkChannel::YAxis, true);

    QSignalSpy spy(&lg, &LinkGroup::yRangeChanged);
    lg.propagateYRange(0, -5.0, 5.0);
    REQUIRE(spy.count() == 1);
}

TEST_CASE("LinkGroup: propagateCrosshair emits when enabled", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.addPanel(0);

    QSignalSpy spy(&lg, &LinkGroup::crosshairMoved);
    lg.propagateCrosshair(0, 42.5);
    REQUIRE(spy.count() == 1);
    REQUIRE(spy.at(0).at(1).toDouble() == 42.5);
}

TEST_CASE("LinkGroup: propagateSelection emits when enabled", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.addPanel(0);
    lg.setChannelEnabled(LinkChannel::Selection, true);

    QSignalSpy spy(&lg, &LinkGroup::selectionChanged);
    lg.propagateSelection(0, 1.0, 5.0);
    REQUIRE(spy.count() == 1);
}

TEST_CASE("LinkGroup: propagate ignores non-member panel", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.addPanel(0);

    QSignalSpy spy(&lg, &LinkGroup::xRangeChanged);
    lg.propagateXRange(99, 1.0, 10.0);
    REQUIRE(spy.count() == 0);
}

TEST_CASE("LinkGroup: generation counter prevents feedback loop", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.addPanel(0);
    lg.addPanel(1);

    int callCount = 0;
    QObject::connect(&lg, &LinkGroup::xRangeChanged,
                     [&](int source, double xMin, double xMax) {
        ++callCount;
        if (callCount < 100) {
            lg.propagateXRange(1 - source, xMin, xMax);
        }
    });

    lg.propagateXRange(0, 1.0, 10.0);
    REQUIRE(callCount < 5);
}

TEST_CASE("LinkGroup: multiple sequential propagations work", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.addPanel(0);

    QSignalSpy spy(&lg, &LinkGroup::xRangeChanged);
    lg.propagateXRange(0, 1.0, 2.0);
    lg.propagateXRange(0, 3.0, 4.0);
    lg.propagateXRange(0, 5.0, 6.0);
    REQUIRE(spy.count() == 3);
}

TEST_CASE("LinkGroup: panels list returns all members", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.addPanel(3);
    lg.addPanel(7);
    lg.addPanel(1);

    auto p = lg.panels();
    REQUIRE(p.size() == 3);
    REQUIRE(p.contains(3));
    REQUIRE(p.contains(7));
    REQUIRE(p.contains(1));
}

TEST_CASE("LinkGroup: removePanel for non-member is no-op", "[dashboard][link]")
{
    LinkGroup lg(QStringLiteral("test"));
    lg.addPanel(0);
    lg.removePanel(99);
    REQUIRE(lg.panels().size() == 1);
}
