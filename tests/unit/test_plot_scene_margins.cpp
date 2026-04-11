#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <memory>
#include <plot/Axis.h>
#include <plot/LineSeries.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>
#include <style/DesignTokens.h>

#include <QFont>
#include <QFontMetrics>

using namespace lumen::plot;
using namespace lumen::data;
using namespace lumen::tokens;

namespace {

/// Create default font metrics matching what computePlotArea uses.
struct DefaultFonts {
    QFont tickFont;
    QFont labelFont;
    QFont titleFont;
    QFontMetrics tickFm;
    QFontMetrics labelFm;
    QFontMetrics titleFm;

    DefaultFonts()
        : tickFont([] {
            QFont f;
            f.setPixelSize(typography::footnote.sizePx);
            f.setWeight(typography::footnote.weight);
            return f;
        }())
        , labelFont([] {
            QFont f;
            f.setPixelSize(typography::bodyStrong.sizePx);
            f.setWeight(typography::bodyStrong.weight);
            return f;
        }())
        , titleFont([] {
            QFont f;
            f.setPixelSize(typography::title3.sizePx);
            f.setWeight(typography::title3.weight);
            return f;
        }())
        , tickFm(tickFont)
        , labelFm(labelFont)
        , titleFm(titleFont)
    {}
};

}  // namespace

TEST_CASE("computeMargins: default margins without labels are reasonable", "[plotscene][margins]") {
    PlotScene scene;
    // Default 0-1 range, no labels, no title.
    DefaultFonts fonts;

    auto margins = scene.computeMargins(fonts.tickFm, fonts.labelFm, fonts.titleFm);

    // Without labels/title, margins should be smaller than the old hardcoded 60/50/30.
    // Left should be much less than 60 (just tick label widths for "0.0"-"1.0" range + spacing).
    CHECK(margins.left < 60.0);
    // Bottom should be less than 50 (just tick height + spacing, no label).
    CHECK(margins.bottom < 50.0);
    // Top should be small padding since no title.
    CHECK(margins.top == spacing::sm);
    // Right is just spacing::md.
    CHECK(margins.right == spacing::md);
    // All margins must be positive.
    CHECK(margins.left > 0.0);
    CHECK(margins.bottom > 0.0);
    CHECK(margins.top > 0.0);
    CHECK(margins.right > 0.0);
}

TEST_CASE("computeMargins: long Y tick labels produce wider left margin", "[plotscene][margins]") {
    DefaultFonts fonts;

    // Scene with small range: ticks like "0.0", "0.5", "1.0".
    PlotScene sceneSmall;
    auto marginsSmall = sceneSmall.computeMargins(fonts.tickFm, fonts.labelFm, fonts.titleFm);

    // Scene with large range: ticks like "0", "200000", "400000", etc.
    PlotScene sceneLarge;
    sceneLarge.yAxis().setRange(0.0, 1000000.0);
    auto marginsLarge = sceneLarge.computeMargins(fonts.tickFm, fonts.labelFm, fonts.titleFm);

    CHECK(marginsLarge.left > marginsSmall.left);
}

TEST_CASE("computeMargins: title present produces taller top margin", "[plotscene][margins]") {
    DefaultFonts fonts;

    PlotScene sceneNoTitle;
    auto marginsNoTitle = sceneNoTitle.computeMargins(fonts.tickFm, fonts.labelFm, fonts.titleFm);

    PlotScene sceneWithTitle;
    sceneWithTitle.setTitle("My Plot Title");
    auto marginsWithTitle = sceneWithTitle.computeMargins(fonts.tickFm, fonts.labelFm, fonts.titleFm);

    CHECK(marginsWithTitle.top > marginsNoTitle.top);
}

TEST_CASE("computeMargins: no title produces small top margin", "[plotscene][margins]") {
    DefaultFonts fonts;

    PlotScene scene;
    auto margins = scene.computeMargins(fonts.tickFm, fonts.labelFm, fonts.titleFm);

    CHECK(margins.top == static_cast<double>(spacing::sm));
}

TEST_CASE("computeMargins: axis labels increase margins", "[plotscene][margins]") {
    DefaultFonts fonts;

    PlotScene sceneNoLabels;
    auto marginsNoLabels = sceneNoLabels.computeMargins(fonts.tickFm, fonts.labelFm, fonts.titleFm);

    PlotScene sceneWithLabels;
    sceneWithLabels.xAxis().setLabel("Time (s)");
    sceneWithLabels.yAxis().setLabel("Voltage (mV)");
    auto marginsWithLabels = sceneWithLabels.computeMargins(fonts.tickFm, fonts.labelFm, fonts.titleFm);

    CHECK(marginsWithLabels.left > marginsNoLabels.left);
    CHECK(marginsWithLabels.bottom > marginsNoLabels.bottom);
}

TEST_CASE("computeMargins: debounce reuses cached margins for sub-pixel changes", "[plotscene][margins]") {
    DefaultFonts fonts;

    PlotScene scene;
    scene.yAxis().setRange(0.0, 100.0);

    // First call: establish cache.
    auto margins1 = scene.computeMargins(fonts.tickFm, fonts.labelFm, fonts.titleFm);

    // Slightly change the range so tick labels change very slightly.
    // For a range of 0-100 vs 0-100.5, the tick labels should be nearly identical.
    scene.yAxis().setRange(0.0, 100.5);
    auto margins2 = scene.computeMargins(fonts.tickFm, fonts.labelFm, fonts.titleFm);

    // The debounce should return the cached value since the difference is <= 1px.
    CHECK(margins1.left == margins2.left);
    CHECK(margins1.top == margins2.top);
    CHECK(margins1.right == margins2.right);
    CHECK(margins1.bottom == margins2.bottom);
}

TEST_CASE("computePlotArea: title affects plot area position", "[plotscene][margins]") {
    QSizeF widgetSize(800.0, 600.0);

    PlotScene sceneNoTitle;
    auto areaNoTitle = sceneNoTitle.computePlotArea(widgetSize);

    PlotScene sceneWithTitle;
    sceneWithTitle.setTitle("Test Title");
    auto areaWithTitle = sceneWithTitle.computePlotArea(widgetSize);

    // With title, plot area should start lower.
    CHECK(areaWithTitle.y() > areaNoTitle.y());
}

TEST_CASE("computePlotArea: plot area has positive dimensions", "[plotscene][margins]") {
    PlotScene scene;
    scene.setTitle("Title");
    scene.xAxis().setLabel("X Label");
    scene.yAxis().setLabel("Y Label");
    scene.yAxis().setRange(0.0, 1000000.0);

    QSizeF widgetSize(800.0, 600.0);
    auto area = scene.computePlotArea(widgetSize);

    CHECK(area.width() > 0.0);
    CHECK(area.height() > 0.0);
    CHECK(area.x() > 0.0);
    CHECK(area.y() > 0.0);
}
