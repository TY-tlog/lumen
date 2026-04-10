#include <catch2/catch_test_macros.hpp>

#include "core/io/WorkspaceFile.h"
#include "data/Column.h"
#include "data/DataFrame.h"
#include "plot/LineSeries.h"
#include "plot/PlotScene.h"
#include "plot/PlotStyle.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

using lumen::core::io::WorkspaceFile;
using lumen::data::Column;
using lumen::data::DataFrame;
using lumen::plot::Axis;
using lumen::plot::Legend;
using lumen::plot::LegendPosition;
using lumen::plot::LineSeries;
using lumen::plot::PlotScene;
using lumen::plot::PlotStyle;
using lumen::plot::RangeMode;
using lumen::plot::TickFormat;

namespace {

/// Helper: build a minimal PlotScene with one series.
struct TestSceneFixture {
    Column xCol{"time", std::vector<double>{1.0, 2.0, 3.0}};
    Column yCol{"voltage", std::vector<double>{10.0, 20.0, 30.0}};
    PlotScene scene;

    TestSceneFixture()
    {
        PlotStyle style;
        style.color = QColor(0x0a, 0x84, 0xff);
        style.lineWidth = 2.0;
        style.penStyle = Qt::DashLine;
        scene.addSeries(LineSeries(&xCol, &yCol, style, "voltage_mV"));
    }
};

/// Helper: build a DataFrame matching the test columns.
std::unique_ptr<DataFrame> makeTestDataFrame()
{
    std::vector<Column> cols;
    cols.emplace_back("time", std::vector<double>{1.0, 2.0, 3.0});
    cols.emplace_back("voltage", std::vector<double>{10.0, 20.0, 30.0});
    return std::make_unique<DataFrame>(std::move(cols));
}

}  // namespace

TEST_CASE("WorkspaceFile: save and load roundtrip", "[core][io][workspace_file]") {
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());
    const QString path = tmpDir.path() + "/test.lumen.json";

    TestSceneFixture fix;
    fix.scene.setTitle("Test Plot");
    fix.scene.setTitleFontPx(20);
    fix.scene.setTitleWeight(QFont::Bold);
    fix.scene.viewTransform().setBaseRange(0.0, 100.0, -50.0, 50.0);
    fix.scene.xAxis().setLabel("Time (ms)");
    fix.scene.xAxis().setRangeMode(RangeMode::Manual);
    fix.scene.xAxis().setManualRange(5.0, 95.0);
    fix.scene.xAxis().setTickCount(10);
    fix.scene.xAxis().setTickFormat(TickFormat::Fixed);
    fix.scene.xAxis().setTickFormatDecimals(3);
    fix.scene.xAxis().setGridVisible(false);
    fix.scene.yAxis().setLabel("Voltage (mV)");
    fix.scene.legend().setPosition(LegendPosition::BottomLeft);
    fix.scene.legend().setVisible(false);

    // Capture and save.
    WorkspaceFile ws = WorkspaceFile::captureFromScene(&fix.scene);
    REQUIRE(ws.isValid());
    REQUIRE(ws.version() == 1);
    ws.saveToPath(path);

    // Load back.
    WorkspaceFile loaded = WorkspaceFile::loadFromPath(path);
    REQUIRE(loaded.isValid());
    REQUIRE(loaded.version() == 1);

    // Apply to a fresh scene.
    auto df = makeTestDataFrame();
    PlotScene fresh;
    loaded.applyToScene(&fresh, df.get());

    // Verify roundtrip.
    CHECK(fresh.title() == "Test Plot");
    CHECK(fresh.titleFontPx() == 20);
    CHECK(fresh.titleWeight() == QFont::Bold);
    CHECK(fresh.viewTransform().xMin() == 0.0);
    CHECK(fresh.viewTransform().xMax() == 100.0);
    CHECK(fresh.viewTransform().yMin() == -50.0);
    CHECK(fresh.viewTransform().yMax() == 50.0);
    CHECK(fresh.xAxis().label() == "Time (ms)");
    CHECK(fresh.xAxis().rangeMode() == RangeMode::Manual);
    CHECK(fresh.xAxis().manualMin() == 5.0);
    CHECK(fresh.xAxis().manualMax() == 95.0);
    CHECK(fresh.xAxis().tickCount() == 10);
    CHECK(fresh.xAxis().tickFormat() == TickFormat::Fixed);
    CHECK(fresh.xAxis().tickFormatDecimals() == 3);
    CHECK(fresh.xAxis().gridVisible() == false);
    CHECK(fresh.yAxis().label() == "Voltage (mV)");
    CHECK(fresh.legend().position() == LegendPosition::BottomLeft);
    CHECK(fresh.legend().isVisible() == false);
    REQUIRE(fresh.seriesCount() == 1);
    CHECK(fresh.series()[0].name() == "voltage_mV");
    CHECK(fresh.series()[0].style().color == QColor(0x0a, 0x84, 0xff));
    CHECK(fresh.series()[0].style().lineWidth == 2.0);
    CHECK(fresh.series()[0].style().penStyle == Qt::DashLine);
}

TEST_CASE("WorkspaceFile: captureFromScene captures viewport",
          "[core][io][workspace_file]") {
    PlotScene scene;
    scene.viewTransform().setBaseRange(10.0, 200.0, -30.0, 70.0);

    WorkspaceFile ws = WorkspaceFile::captureFromScene(&scene);
    REQUIRE(ws.isValid());

    const auto plotObj = ws.data()["plot"].toObject();
    const auto vp = plotObj["viewport"].toObject();
    CHECK(vp["xmin"].toDouble() == 10.0);
    CHECK(vp["xmax"].toDouble() == 200.0);
    CHECK(vp["ymin"].toDouble() == -30.0);
    CHECK(vp["ymax"].toDouble() == 70.0);
}

TEST_CASE("WorkspaceFile: captureFromScene captures series",
          "[core][io][workspace_file]") {
    TestSceneFixture fix;
    fix.scene.seriesAt(0).setVisible(false);

    WorkspaceFile ws = WorkspaceFile::captureFromScene(&fix.scene);
    REQUIRE(ws.isValid());

    const auto plotObj = ws.data()["plot"].toObject();
    const auto seriesArr = plotObj["series"].toArray();
    REQUIRE(seriesArr.size() == 1);

    const auto s0 = seriesArr[0].toObject();
    CHECK(s0["xColumn"].toString() == "time");
    CHECK(s0["yColumn"].toString() == "voltage");
    CHECK(s0["color"].toString() == "#0a84ff");
    CHECK(s0["lineWidth"].toDouble() == 2.0);
    CHECK(s0["lineStyle"].toString() == "dash");
    CHECK(s0["name"].toString() == "voltage_mV");
    CHECK(s0["visible"].toBool() == false);
}

TEST_CASE("WorkspaceFile: invalid JSON returns isValid false",
          "[core][io][workspace_file]") {
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());
    const QString path = tmpDir.path() + "/bad.lumen.json";

    // Write garbage.
    QFile file(path);
    REQUIRE(file.open(QIODevice::WriteOnly));
    file.write("{ this is not valid json!!!");
    file.close();

    WorkspaceFile ws = WorkspaceFile::loadFromPath(path);
    CHECK_FALSE(ws.isValid());
    CHECK(ws.version() == 0);
}

TEST_CASE("WorkspaceFile: missing version field returns isValid false",
          "[core][io][workspace_file]") {
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());
    const QString path = tmpDir.path() + "/no_version.lumen.json";

    QJsonObject obj;
    obj["plot"] = QJsonObject();
    // No "version" key.
    QFile file(path);
    REQUIRE(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(obj).toJson());
    file.close();

    WorkspaceFile ws = WorkspaceFile::loadFromPath(path);
    CHECK_FALSE(ws.isValid());
}

TEST_CASE("WorkspaceFile: missing optional fields use defaults",
          "[core][io][workspace_file]") {
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());
    const QString path = tmpDir.path() + "/minimal.lumen.json";

    // Write a minimal valid JSON with only version and empty plot.
    QJsonObject root;
    root["version"] = 1;
    root["plot"] = QJsonObject();  // empty — no viewport, title, etc.
    QFile file(path);
    REQUIRE(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(root).toJson());
    file.close();

    WorkspaceFile ws = WorkspaceFile::loadFromPath(path);
    REQUIRE(ws.isValid());

    // Apply to a scene; should not crash, scene keeps its defaults.
    PlotScene scene;
    scene.setTitle("Original");
    ws.applyToScene(&scene, nullptr);

    // Title was not in the JSON, so it remains as set before apply.
    CHECK(scene.title() == "Original");
}

TEST_CASE("WorkspaceFile: applyToScene restores title",
          "[core][io][workspace_file]") {
    TestSceneFixture fix;
    fix.scene.setTitle("Captured Title");
    fix.scene.setTitleFontPx(24);
    fix.scene.setTitleWeight(QFont::Black);

    WorkspaceFile ws = WorkspaceFile::captureFromScene(&fix.scene);
    REQUIRE(ws.isValid());

    PlotScene target;
    target.setTitle("Old Title");
    target.setTitleFontPx(12);

    auto df = makeTestDataFrame();
    ws.applyToScene(&target, df.get());

    CHECK(target.title() == "Captured Title");
    CHECK(target.titleFontPx() == 24);
    CHECK(target.titleWeight() == QFont::Black);
}

TEST_CASE("WorkspaceFile: nonexistent file returns invalid",
          "[core][io][workspace_file]") {
    WorkspaceFile ws = WorkspaceFile::loadFromPath("/nonexistent/path/to/file.lumen.json");
    CHECK_FALSE(ws.isValid());
    CHECK(ws.version() == 0);
}

TEST_CASE("WorkspaceFile: captureFromScene with null scene returns invalid",
          "[core][io][workspace_file]") {
    WorkspaceFile ws = WorkspaceFile::captureFromScene(nullptr);
    CHECK_FALSE(ws.isValid());
}

TEST_CASE("WorkspaceFile: multiple series roundtrip",
          "[core][io][workspace_file]") {
    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());
    const QString path = tmpDir.path() + "/multi.lumen.json";

    Column xCol{"time", std::vector<double>{1.0, 2.0}};
    Column y1Col{"voltage", std::vector<double>{10.0, 20.0}};
    Column y2Col{"current", std::vector<double>{0.1, 0.2}};

    PlotScene scene;
    PlotStyle s1;
    s1.color = QColor(Qt::red);
    s1.lineWidth = 1.0;
    s1.penStyle = Qt::SolidLine;
    scene.addSeries(LineSeries(&xCol, &y1Col, s1, "Voltage"));

    PlotStyle s2;
    s2.color = QColor(Qt::blue);
    s2.lineWidth = 2.5;
    s2.penStyle = Qt::DotLine;
    scene.addSeries(LineSeries(&xCol, &y2Col, s2, "Current"));

    WorkspaceFile ws = WorkspaceFile::captureFromScene(&scene);
    ws.saveToPath(path);

    WorkspaceFile loaded = WorkspaceFile::loadFromPath(path);
    REQUIRE(loaded.isValid());

    // Build a DataFrame with all three columns.
    std::vector<Column> cols;
    cols.emplace_back("time", std::vector<double>{1.0, 2.0});
    cols.emplace_back("voltage", std::vector<double>{10.0, 20.0});
    cols.emplace_back("current", std::vector<double>{0.1, 0.2});
    DataFrame df(std::move(cols));

    PlotScene target;
    loaded.applyToScene(&target, &df);

    REQUIRE(target.seriesCount() == 2);
    CHECK(target.series()[0].name() == "Voltage");
    CHECK(target.series()[0].style().color == QColor(Qt::red));
    CHECK(target.series()[1].name() == "Current");
    CHECK(target.series()[1].style().color == QColor(Qt::blue));
    CHECK(target.series()[1].style().penStyle == Qt::DotLine);
}
