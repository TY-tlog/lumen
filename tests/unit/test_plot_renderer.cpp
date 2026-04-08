// PlotRenderer tests — render to QImage and verify output.
// Note: QGuiApplication + fontconfig triggers benign LeakSanitizer
// reports from libfontconfig internals (not our code).
// ASAN_OPTIONS=detect_leaks=0 suppresses if needed.

#include <catch2/catch_test_macros.hpp>

#include <data/Column.h>
#include <plot/LineSeries.h>
#include <plot/PlotRenderer.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>
#include <style/DesignTokens.h>

#include <QGuiApplication>
#include <QImage>
#include <QPainter>

using namespace lumen::plot;
using namespace lumen::data;

namespace {
// Ensure QGuiApplication exists for QPainter/QImage.
struct AppGuard {
    AppGuard() {
        if (QGuiApplication::instance() == nullptr) {
            static int argc = 1;
            static char arg0[] = "test";
            static char* argv[] = {arg0, nullptr};
            app = new QGuiApplication(argc, argv);
        }
    }
    QGuiApplication* app = nullptr;
};
static AppGuard guard;
}  // namespace

TEST_CASE("PlotRenderer renders background", "[plotrenderer]") {
    PlotScene scene;
    PlotRenderer renderer;

    QImage image(400, 300, QImage::Format_ARGB32);
    image.fill(Qt::black);

    QPainter painter(&image);
    renderer.render(painter, scene, QSizeF(400, 300));
    painter.end();

    // Corner pixel should be background.primary (white).
    QColor corner = image.pixelColor(0, 0);
    REQUIRE(corner == lumen::tokens::color::background::primary);
}

TEST_CASE("PlotRenderer renders non-empty image with series", "[plotrenderer]") {
    std::vector<double> x = {0.0, 1.0, 2.0, 3.0, 4.0};
    std::vector<double> y = {0.0, 10.0, 5.0, 15.0, 3.0};
    Column xCol("x", x);
    Column yCol("y", y);

    PlotScene scene;
    scene.addSeries(LineSeries(&xCol, &yCol, PlotStyle::fromPalette(0), "test"));
    scene.autoRange();
    scene.setTitle("Test Plot");

    PlotRenderer renderer;
    QImage image(400, 300, QImage::Format_ARGB32);
    image.fill(Qt::black);

    QPainter painter(&image);
    renderer.render(painter, scene, QSizeF(400, 300));
    painter.end();

    // Image should not be all black (rendering happened).
    bool allBlack = true;
    for (int py = 0; py < image.height() && allBlack; ++py) {
        for (int px = 0; px < image.width() && allBlack; ++px) {
            if (image.pixelColor(px, py) != QColor(Qt::black)) {
                allBlack = false;
            }
        }
    }
    REQUIRE_FALSE(allBlack);

    // Image should not be all white either (axes, lines drawn).
    bool allWhite = true;
    for (int py = 0; py < image.height() && allWhite; ++py) {
        for (int px = 0; px < image.width() && allWhite; ++px) {
            if (image.pixelColor(px, py) != lumen::tokens::color::background::primary) {
                allWhite = false;
            }
        }
    }
    REQUIRE_FALSE(allWhite);
}

TEST_CASE("PlotRenderer handles empty scene", "[plotrenderer]") {
    PlotScene scene;
    PlotRenderer renderer;

    QImage image(200, 150, QImage::Format_ARGB32);
    QPainter painter(&image);

    // Should not crash with no series.
    renderer.render(painter, scene, QSizeF(200, 150));
    painter.end();

    QColor corner = image.pixelColor(0, 0);
    REQUIRE(corner == lumen::tokens::color::background::primary);
}
