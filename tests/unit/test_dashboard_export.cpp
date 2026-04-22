#include <catch2/catch_test_macros.hpp>

#include <core/io/FigureExporter.h>
#include <dashboard/Dashboard.h>
#include <data/Rank1Dataset.h>
#include <data/Unit.h>
#include <plot/LineSeries.h>
#include <plot/PlotScene.h>
#include <plot/PlotStyle.h>

#include <QImage>
#include <QTemporaryDir>

#include <cmath>
#include <memory>

using namespace lumen::core::io;
using namespace lumen::dashboard;

namespace {
PanelConfig pc(int row = 0, int col = 0)
{
    PanelConfig c;
    c.row = row;
    c.col = col;
    return c;
}

void addSine(lumen::plot::PlotScene* scene)
{
    std::vector<double> xd, yd;
    for (int i = 0; i < 50; ++i) {
        xd.push_back(static_cast<double>(i) * 0.1);
        yd.push_back(std::sin(xd.back()));
    }
    auto xDs = std::make_shared<lumen::data::Rank1Dataset>(
        "x", lumen::data::Unit::dimensionless(), std::move(xd));
    auto yDs = std::make_shared<lumen::data::Rank1Dataset>(
        "y", lumen::data::Unit::dimensionless(), std::move(yd));
    lumen::plot::PlotStyle style;
    style.color = QColor(Qt::blue);
    auto series = std::make_unique<lumen::plot::LineSeries>(
        xDs, yDs, style, QStringLiteral("sine"));
    scene->addItem(std::move(series));
    scene->autoRange();
}
}  // namespace

TEST_CASE("exportDashboard: empty dashboard returns error", "[dashboard][export]")
{
    Dashboard db;
    FigureExporter::Options opts;
    opts.outputPath = QStringLiteral("/tmp/test.png");
    REQUIRE(!FigureExporter::exportDashboard(&db, opts).isEmpty());
}

TEST_CASE("exportDashboard: produces non-empty PNG", "[dashboard][export]")
{
    Dashboard db;
    db.setGridSize(1, 2);
    db.addPanel(pc(0, 0));
    db.addPanel(pc(0, 1));
    addSine(db.sceneAt(0));
    addSine(db.sceneAt(1));

    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());

    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Png;
    opts.outputPath = tmpDir.filePath("dashboard.png");
    opts.widthPx = 800;
    opts.heightPx = 400;

    QString err = FigureExporter::exportDashboard(&db, opts);
    REQUIRE(err.isEmpty());

    QImage img(opts.outputPath);
    REQUIRE(!img.isNull());
    REQUIRE(img.width() == 800);
    REQUIRE(img.height() == 400);
}

TEST_CASE("exportDashboard: 2x2 grid renders all panels", "[dashboard][export]")
{
    Dashboard db;
    db.setGridSize(2, 2);
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 2; ++c) {
            db.addPanel(pc(r, c));
            addSine(db.sceneAt(db.panelCount() - 1));
        }

    QTemporaryDir tmpDir;
    FigureExporter::Options opts;
    opts.format = FigureExporter::Format::Png;
    opts.outputPath = tmpDir.filePath("grid.png");
    opts.widthPx = 800;
    opts.heightPx = 600;

    QString err = FigureExporter::exportDashboard(&db, opts);
    REQUIRE(err.isEmpty());

    QImage img(opts.outputPath);
    REQUIRE(!img.isNull());

    // Check that all four quadrants have non-white content.
    auto quadrant = [&](int qx, int qy) {
        int x0 = qx * 400, y0 = qy * 300;
        bool hasContent = false;
        for (int y = y0 + 10; y < y0 + 290 && !hasContent; ++y)
            for (int x = x0 + 10; x < x0 + 390 && !hasContent; ++x)
                if (img.pixelColor(x, y) != QColor(Qt::white))
                    hasContent = true;
        return hasContent;
    };
    REQUIRE(quadrant(0, 0));
    REQUIRE(quadrant(1, 0));
    REQUIRE(quadrant(0, 1));
    REQUIRE(quadrant(1, 1));
}
