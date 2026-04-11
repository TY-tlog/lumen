#include <catch2/catch_test_macros.hpp>

#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <plot/CoordinateMapper.h>
#include <plot/HitTester.h>
#include <plot/LineSeries.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>

#include <memory>

using namespace lumen::plot;
using namespace lumen::data;

namespace {

constexpr QSizeF kWidgetSize{800.0, 600.0};

struct TitleLegendFixture {
    std::shared_ptr<Rank1Dataset> xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{0.0, 10.0});
    std::shared_ptr<Rank1Dataset> y0Ds = std::make_shared<Rank1Dataset>("y0", Unit::dimensionless(), std::vector<double>{0.0, 10.0});
    std::shared_ptr<Rank1Dataset> y1Ds = std::make_shared<Rank1Dataset>("y1", Unit::dimensionless(), std::vector<double>{5.0, 15.0});
    std::unique_ptr<PlotScene> scene = std::make_unique<PlotScene>();

    TitleLegendFixture() {
        scene->setTitle("Test Plot");
        scene->addSeries(LineSeries(xDs, y0Ds, PlotStyle::fromPalette(0), "Series A"));
        scene->addSeries(LineSeries(xDs, y1Ds, PlotStyle::fromPalette(1), "Series B"));
        scene->autoRange();
    }
};

struct NoTitleFixture {
    std::shared_ptr<Rank1Dataset> xDs = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), std::vector<double>{0.0, 10.0});
    std::shared_ptr<Rank1Dataset> yDs = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), std::vector<double>{0.0, 10.0});
    std::unique_ptr<PlotScene> scene = std::make_unique<PlotScene>();

    NoTitleFixture() {
        scene->addSeries(LineSeries(xDs, yDs, PlotStyle::fromPalette(0), "Only"));
        scene->autoRange();
    }
};

}  // namespace

TEST_CASE("hitNonSeriesElement: point in title area returns Title", "[hittester][regions]") {
    TitleLegendFixture fix;
    QRectF plotArea = fix.scene->computePlotArea(kWidgetSize);
    QPointF titlePoint(plotArea.center().x(), plotArea.top() / 2.0);
    auto result = HitTester::hitNonSeriesElement(*fix.scene, kWidgetSize, titlePoint);
    CHECK(result.kind == HitKind::Title);
}

TEST_CASE("hitNonSeriesElement: point in X axis band returns XAxis", "[hittester][regions]") {
    TitleLegendFixture fix;
    QRectF plotArea = fix.scene->computePlotArea(kWidgetSize);
    QPointF xAxisPoint(plotArea.center().x(), plotArea.bottom() + 10.0);
    auto result = HitTester::hitNonSeriesElement(*fix.scene, kWidgetSize, xAxisPoint);
    CHECK(result.kind == HitKind::XAxis);
}

TEST_CASE("hitNonSeriesElement: point in Y axis band returns YAxis", "[hittester][regions]") {
    TitleLegendFixture fix;
    QRectF plotArea = fix.scene->computePlotArea(kWidgetSize);
    QPointF yAxisPoint(plotArea.left() / 2.0, plotArea.center().y());
    auto result = HitTester::hitNonSeriesElement(*fix.scene, kWidgetSize, yAxisPoint);
    CHECK(result.kind == HitKind::YAxis);
}

TEST_CASE("hitNonSeriesElement: point inside plot area returns PlotArea", "[hittester][regions]") {
    NoTitleFixture fix;
    QRectF plotArea = fix.scene->computePlotArea(kWidgetSize);
    QPointF centerPoint = plotArea.center();
    auto result = HitTester::hitNonSeriesElement(*fix.scene, kWidgetSize, centerPoint);
    CHECK(result.kind == HitKind::PlotArea);
}

TEST_CASE("hitNonSeriesElement: point in legend area returns Legend", "[hittester][regions]") {
    TitleLegendFixture fix;
    QRectF plotArea = fix.scene->computePlotArea(kWidgetSize);
    QPointF legendPoint(plotArea.right() - 20.0, plotArea.top() + 20.0);
    auto result = HitTester::hitNonSeriesElement(*fix.scene, kWidgetSize, legendPoint);
    CHECK(result.kind == HitKind::Legend);
}

TEST_CASE("hitNonSeriesElement: precedence -- title wins over Y axis overlap", "[hittester][regions]") {
    TitleLegendFixture fix;
    QRectF plotArea = fix.scene->computePlotArea(kWidgetSize);
    QPointF overlapPoint(plotArea.left() + 1.0, plotArea.top() / 2.0);
    auto result = HitTester::hitNonSeriesElement(*fix.scene, kWidgetSize, overlapPoint);
    CHECK(result.kind == HitKind::Title);
}

TEST_CASE("hitNonSeriesElement: no title -- title area returns None or axis", "[hittester][regions]") {
    NoTitleFixture fix;
    QRectF plotArea = fix.scene->computePlotArea(kWidgetSize);
    QPointF abovePoint(plotArea.center().x(), 2.0);
    auto result = HitTester::hitNonSeriesElement(*fix.scene, kWidgetSize, abovePoint);
    CHECK(result.kind == HitKind::None);
}

TEST_CASE("hitNonSeriesElement: point outside all regions returns None", "[hittester][regions]") {
    NoTitleFixture fix;
    QPointF outside(-50.0, -50.0);
    auto result = HitTester::hitNonSeriesElement(*fix.scene, kWidgetSize, outside);
    CHECK(result.kind == HitKind::None);
}
