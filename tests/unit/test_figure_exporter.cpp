// FigureExporter tests — export PlotScene to PNG/SVG/PDF files.
// Validates file creation, content correctness, and error handling.
// ASAN_OPTIONS=detect_leaks=0 suppresses benign fontconfig leak reports.

#include <catch2/catch_test_macros.hpp>

#include <core/io/FigureExporter.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <memory>
#include <plot/LineSeries.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QImage>

using namespace lumen::core::io;
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

/// Populate a PlotScene with a 5-point line series for testing.
/// PlotScene is non-copyable/non-movable (QObject members), so we
/// configure it in place rather than returning by value.
void populateScene(PlotScene& scene, std::shared_ptr<Rank1Dataset> xDs, std::shared_ptr<Rank1Dataset> yDs) {
    scene.addSeries(LineSeries(xDs, yDs, PlotStyle::fromPalette(0), "Test Series"));
    scene.autoRange();
    scene.setTitle("Test Plot");
}

/// Return a temporary file path for the given extension.
QString tempPath(const QString& ext) {
    return QDir::tempPath() + "/lumen_test_export." + ext;
}

/// Remove a file if it exists (cleanup helper).
void removeIfExists(const QString& path) {
    QFile::remove(path);
}

}  // namespace

// ---- Test 1: PNG export creates file at specified path ----
TEST_CASE("FigureExporter PNG creates file at path", "[figureexporter]") {
    std::vector<double> x = {0.0, 1.0, 2.0, 3.0, 4.0};
    std::vector<double> y = {0.0, 10.0, 5.0, 15.0, 3.0};
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), x);
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), y);
    PlotScene scene;
    populateScene(scene, xCol, yCol);

    QString path = tempPath("png");
    removeIfExists(path);

    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Png;
    opts.widthPx = 400;
    opts.heightPx = 300;
    opts.dpi = 150;
    opts.outputPath = path;

    QString err = FigureExporter::exportFigure(&scene, opts);
    REQUIRE(err.isEmpty());
    REQUIRE(QFile::exists(path));

    removeIfExists(path);
}

// ---- Test 2: PNG file has correct dimensions ----
TEST_CASE("FigureExporter PNG has correct dimensions", "[figureexporter]") {
    std::vector<double> x = {0.0, 1.0, 2.0, 3.0, 4.0};
    std::vector<double> y = {0.0, 10.0, 5.0, 15.0, 3.0};
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), x);
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), y);
    PlotScene scene;
    populateScene(scene, xCol, yCol);

    QString path = tempPath("png");
    removeIfExists(path);

    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Png;
    opts.widthPx = 800;
    opts.heightPx = 600;
    opts.dpi = 300;
    opts.outputPath = path;

    QString err = FigureExporter::exportFigure(&scene, opts);
    REQUIRE(err.isEmpty());

    QImage img(path);
    REQUIRE(img.width() == 800);
    REQUIRE(img.height() == 600);

    removeIfExists(path);
}

// ---- Test 3: PNG at different DPI sets correct metadata ----
TEST_CASE("FigureExporter PNG DPI metadata is correct", "[figureexporter]") {
    std::vector<double> x = {0.0, 1.0, 2.0};
    std::vector<double> y = {0.0, 5.0, 10.0};
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), x);
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), y);
    PlotScene scene;
    populateScene(scene, xCol, yCol);

    QString path = tempPath("png");
    removeIfExists(path);

    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Png;
    opts.widthPx = 400;
    opts.heightPx = 300;
    opts.dpi = 600;
    opts.outputPath = path;

    QString err = FigureExporter::exportFigure(&scene, opts);
    REQUIRE(err.isEmpty());

    QImage img(path);
    // 600 DPI = 600 / 0.0254 = ~23622 dots per meter.
    const int expectedDpm = static_cast<int>(600 / 0.0254);
    // Allow small rounding difference.
    REQUIRE(std::abs(img.dotsPerMeterX() - expectedDpm) <= 1);
    REQUIRE(std::abs(img.dotsPerMeterY() - expectedDpm) <= 1);

    removeIfExists(path);
}

// ---- Test 4: SVG export creates file containing "<svg" ----
TEST_CASE("FigureExporter SVG creates valid SVG file", "[figureexporter]") {
    std::vector<double> x = {0.0, 1.0, 2.0, 3.0, 4.0};
    std::vector<double> y = {0.0, 10.0, 5.0, 15.0, 3.0};
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), x);
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), y);
    PlotScene scene;
    populateScene(scene, xCol, yCol);

    QString path = tempPath("svg");
    removeIfExists(path);

    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Svg;
    opts.widthPx = 400;
    opts.heightPx = 300;
    opts.outputPath = path;

    QString err = FigureExporter::exportFigure(&scene, opts);
    REQUIRE(err.isEmpty());
    REQUIRE(QFile::exists(path));

    QFile file(path);
    REQUIRE(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QByteArray content = file.readAll();
    REQUIRE(content.contains("<svg"));

    removeIfExists(path);
}

// ---- Test 5: PDF export creates file starting with "%PDF-" ----
TEST_CASE("FigureExporter PDF creates valid PDF file", "[figureexporter]") {
    std::vector<double> x = {0.0, 1.0, 2.0, 3.0, 4.0};
    std::vector<double> y = {0.0, 10.0, 5.0, 15.0, 3.0};
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), x);
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), y);
    PlotScene scene;
    populateScene(scene, xCol, yCol);

    QString path = tempPath("pdf");
    removeIfExists(path);

    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Pdf;
    opts.widthPx = 400;
    opts.heightPx = 300;
    opts.dpi = 300;
    opts.outputPath = path;

    QString err = FigureExporter::exportFigure(&scene, opts);
    REQUIRE(err.isEmpty());
    REQUIRE(QFile::exists(path));

    QFile file(path);
    REQUIRE(file.open(QIODevice::ReadOnly));
    QByteArray header = file.read(5);
    REQUIRE(header == "%PDF-");

    removeIfExists(path);
}

// ---- Test 6: Transparent PNG has alpha < 255 at background pixel ----
TEST_CASE("FigureExporter transparent PNG has transparent background", "[figureexporter]") {
    std::vector<double> x = {0.0, 1.0, 2.0, 3.0, 4.0};
    std::vector<double> y = {0.0, 10.0, 5.0, 15.0, 3.0};
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), x);
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), y);
    PlotScene scene;
    populateScene(scene, xCol, yCol);

    QString path = tempPath("png");
    removeIfExists(path);

    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Png;
    opts.widthPx = 400;
    opts.heightPx = 300;
    opts.dpi = 150;
    opts.transparentBackground = true;
    opts.outputPath = path;

    QString err = FigureExporter::exportFigure(&scene, opts);
    REQUIRE(err.isEmpty());

    QImage img(path);
    REQUIRE(img.format() == QImage::Format_ARGB32);

    // Pixel at (0,0) is top-left corner — should be transparent
    // (no series data drawn there).
    QColor corner = img.pixelColor(0, 0);
    REQUIRE(corner.alpha() < 255);

    removeIfExists(path);
}

// ---- Test 7: Export does not modify PlotScene ----
TEST_CASE("FigureExporter does not modify PlotScene", "[figureexporter]") {
    std::vector<double> x = {0.0, 1.0, 2.0, 3.0, 4.0};
    std::vector<double> y = {0.0, 10.0, 5.0, 15.0, 3.0};
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), x);
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), y);
    PlotScene scene;
    populateScene(scene, xCol, yCol);

    // Capture state before export.
    const QString titleBefore = scene.title();
    const std::size_t seriesCountBefore = scene.seriesCount();
    const double xMinBefore = scene.viewTransform().xMin();
    const double xMaxBefore = scene.viewTransform().xMax();

    QString path = tempPath("png");
    removeIfExists(path);

    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Png;
    opts.widthPx = 400;
    opts.heightPx = 300;
    opts.outputPath = path;

    FigureExporter::exportFigure(&scene, opts);

    // Verify scene is unchanged.
    REQUIRE(scene.title() == titleBefore);
    REQUIRE(scene.seriesCount() == seriesCountBefore);
    REQUIRE(scene.viewTransform().xMin() == xMinBefore);
    REQUIRE(scene.viewTransform().xMax() == xMaxBefore);

    removeIfExists(path);
}

// ---- Test 8: Invalid path returns non-empty error string ----
TEST_CASE("FigureExporter returns error for invalid path", "[figureexporter]") {
    std::vector<double> x = {0.0, 1.0};
    std::vector<double> y = {0.0, 1.0};
    auto xCol = std::make_shared<Rank1Dataset>("x", Unit::dimensionless(), x);
    auto yCol = std::make_shared<Rank1Dataset>("y", Unit::dimensionless(), y);
    PlotScene scene;
    populateScene(scene, xCol, yCol);

    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Png;
    opts.widthPx = 100;
    opts.heightPx = 100;
    opts.outputPath = "/nonexistent/deeply/nested/path/test.png";

    QString err = FigureExporter::exportFigure(&scene, opts);
    REQUIRE_FALSE(err.isEmpty());
}

// ---- Test 9: Empty scene (no series) exports without crash ----
TEST_CASE("FigureExporter exports empty scene without crash", "[figureexporter]") {
    PlotScene scene;

    QString path = tempPath("png");
    removeIfExists(path);

    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Png;
    opts.widthPx = 200;
    opts.heightPx = 150;
    opts.outputPath = path;

    QString err = FigureExporter::exportFigure(&scene, opts);
    REQUIRE(err.isEmpty());
    REQUIRE(QFile::exists(path));

    removeIfExists(path);
}

// ---- Test 10: Empty output path returns error ----
TEST_CASE("FigureExporter returns error for empty output path", "[figureexporter]") {
    PlotScene scene;

    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Png;
    opts.outputPath = "";

    QString err = FigureExporter::exportFigure(&scene, opts);
    REQUIRE_FALSE(err.isEmpty());
}

// ---- Test 11: Null scene returns error ----
TEST_CASE("FigureExporter returns error for null scene", "[figureexporter]") {
    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Png;
    opts.widthPx = 100;
    opts.heightPx = 100;
    opts.outputPath = tempPath("png");

    QString err = FigureExporter::exportFigure(nullptr, opts);
    REQUIRE_FALSE(err.isEmpty());
}
