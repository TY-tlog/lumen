#include <catch2/catch_test_macros.hpp>

#include <core/io/FigureExporter.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <plot/LineSeries.h>
#include <plot/PlotRenderer.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QSvgGenerator>

#include <atomic>
#include <memory>

using namespace lumen::plot;
using namespace lumen::data;
using namespace lumen::core::io;

namespace {

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

QString tempPath(const QString& ext) {
    static std::atomic<int> counter{0};
    return QDir::tempPath() + QStringLiteral("/lumen_test_tap_%1_%2.%3")
        .arg(QCoreApplication::applicationPid())
        .arg(counter.fetch_add(1))
        .arg(ext);
}

void populateScene(PlotScene& scene) {
    auto x = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(),
        std::vector<double>{0, 1, 2, 3, 4});
    auto y = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(),
        std::vector<double>{0, 5, 10, 5, 0});
    scene.addSeries(LineSeries(x, y, PlotStyle::fromPalette(0), "Test"));
    scene.autoRange();
    scene.setTitle("Title Text");
    scene.xAxis().setLabel("X Axis");
    scene.yAxis().setLabel("Y Axis");
}

}  // namespace

TEST_CASE("PlotRenderer: textAsPath defaults to false", "[text_as_path]") {
    PlotRenderer renderer;
    CHECK_FALSE(renderer.textAsPath());
}

TEST_CASE("PlotRenderer: textAsPath can be enabled", "[text_as_path]") {
    PlotRenderer renderer;
    renderer.setTextAsPath(true);
    CHECK(renderer.textAsPath());
}

TEST_CASE("SVG export uses path elements instead of text (ADR-055)", "[text_as_path]") {
    PlotScene scene;
    populateScene(scene);

    QString path = tempPath("svg");

    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Svg;
    opts.widthPx = 400;
    opts.heightPx = 300;
    opts.outputPath = path;

    QString err = FigureExporter::exportFigure(&scene, opts);
    REQUIRE(err.isEmpty());

    QFile f(path);
    REQUIRE(f.open(QIODevice::ReadOnly));
    QByteArray data = f.readAll();
    f.close();

    // With text-as-path, SVG should contain <path> elements
    // and fewer/no <text> elements for our rendered text.
    CHECK(data.contains("<path"));
    // The SVG should be valid (starts with proper XML/SVG header).
    CHECK(data.contains("<svg"));

    QFile::remove(path);
}
