#include <catch2/catch_test_macros.hpp>

#include "core/CommandBus.h"
#include "core/DocumentRegistry.h"
#include "core/PlotRegistry.h"
#include "core/io/WorkspaceFile.h"
#include "core/io/WorkspaceManager.h"
#include "data/Column.h"
#include "data/DataFrame.h"
#include "plot/LineSeries.h"
#include "plot/PlotScene.h"
#include "plot/PlotStyle.h"

#include <QSignalSpy>
#include <QTemporaryDir>

using lumen::core::CommandBus;
using lumen::core::DocumentRegistry;
using lumen::core::PlotRegistry;
using lumen::core::io::WorkspaceFile;
using lumen::core::io::WorkspaceManager;
using lumen::data::Column;
using lumen::data::DataFrame;
using lumen::plot::LineSeries;
using lumen::plot::PlotScene;
using lumen::plot::PlotStyle;

namespace {

/// Trivial command for triggering commandExecuted signals.
class DummyCommand : public lumen::core::Command {
public:
    void execute() override {}
    void undo() override {}
    QString description() const override { return QStringLiteral("dummy"); }
};

struct ManagerFixture {
    DocumentRegistry docs;
    PlotRegistry plots;
    CommandBus bus;
    WorkspaceManager mgr{&docs, &plots, &bus};

    QTemporaryDir tmpDir;
    QString docPath;
    PlotScene scene;
    Column xCol{"time", std::vector<double>{1.0, 2.0, 3.0}};
    Column yCol{"voltage", std::vector<double>{10.0, 20.0, 30.0}};

    ManagerFixture()
    {
        REQUIRE(tmpDir.isValid());
        docPath = tmpDir.path() + "/data.csv";

        // Create a dummy CSV file so the path exists.
        QFile f(docPath);
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write("time,voltage\n1,10\n2,20\n3,30\n");
        f.close();

        // Set up scene with a series.
        PlotStyle style;
        style.color = QColor(Qt::red);
        scene.addSeries(LineSeries(&xCol, &yCol, style, "Voltage"));
        scene.setTitle("My Plot");

        // Register with manager.
        mgr.registerPlotScene(docPath, &scene);
    }
};

}  // namespace

TEST_CASE("WorkspaceManager: modified flag set on commandExecuted",
          "[core][io][workspace_manager]") {
    ManagerFixture fix;

    CHECK_FALSE(fix.mgr.isModified(fix.docPath));

    fix.bus.execute(std::make_unique<DummyCommand>());

    CHECK(fix.mgr.isModified(fix.docPath));
}

TEST_CASE("WorkspaceManager: modified flag cleared on save",
          "[core][io][workspace_manager]") {
    ManagerFixture fix;

    // Mark as modified.
    fix.bus.execute(std::make_unique<DummyCommand>());
    REQUIRE(fix.mgr.isModified(fix.docPath));

    // Save.
    bool ok = fix.mgr.saveWorkspace(fix.docPath);
    CHECK(ok);
    CHECK_FALSE(fix.mgr.isModified(fix.docPath));
}

TEST_CASE("WorkspaceManager: defaultSidecarPath correct",
          "[core][io][workspace_manager]") {
    ManagerFixture fix;

    CHECK(fix.mgr.defaultSidecarPath("/path/to/data.csv") == "/path/to/data.lumen.json");
    CHECK(fix.mgr.defaultSidecarPath("/path/to/data.CSV") == "/path/to/data.lumen.json");
    CHECK(fix.mgr.defaultSidecarPath("/path/to/data.tsv") == "/path/to/data.tsv.lumen.json");
    CHECK(fix.mgr.defaultSidecarPath("/path/to/myfile") == "/path/to/myfile.lumen.json");
}

TEST_CASE("WorkspaceManager: loadWorkspaceIfExists no-op without sidecar",
          "[core][io][workspace_manager]") {
    ManagerFixture fix;

    // No sidecar file exists yet.
    bool loaded = fix.mgr.loadWorkspaceIfExists(fix.docPath);
    CHECK_FALSE(loaded);
}

TEST_CASE("WorkspaceManager: signals emitted on save",
          "[core][io][workspace_manager]") {
    ManagerFixture fix;

    QSignalSpy savedSpy(&fix.mgr, &WorkspaceManager::workspaceSaved);
    QSignalSpy modifiedSpy(&fix.mgr, &WorkspaceManager::modifiedChanged);
    REQUIRE(savedSpy.isValid());
    REQUIRE(modifiedSpy.isValid());

    // Mark modified.
    fix.bus.execute(std::make_unique<DummyCommand>());
    REQUIRE(modifiedSpy.count() == 1);
    CHECK(modifiedSpy.at(0).at(1).toBool() == true);

    // Save.
    fix.mgr.saveWorkspace(fix.docPath);

    CHECK(savedSpy.count() == 1);
    CHECK(savedSpy.at(0).at(0).toString() == fix.docPath);

    // modified should have changed to false.
    CHECK(modifiedSpy.count() == 2);
    CHECK(modifiedSpy.at(1).at(1).toBool() == false);
}

TEST_CASE("WorkspaceManager: save then load roundtrip",
          "[core][io][workspace_manager]") {
    ManagerFixture fix;
    fix.scene.setTitle("Saved Title");

    // Save.
    bool ok = fix.mgr.saveWorkspace(fix.docPath);
    REQUIRE(ok);

    // Verify sidecar file exists.
    const QString sidecar = fix.mgr.defaultSidecarPath(fix.docPath);
    CHECK(QFile::exists(sidecar));

    // Change the scene.
    fix.scene.setTitle("Changed Title");

    // Register DataFrame for column resolution on load.
    std::vector<Column> cols;
    cols.emplace_back("time", std::vector<double>{1.0, 2.0, 3.0});
    cols.emplace_back("voltage", std::vector<double>{10.0, 20.0, 30.0});
    fix.docs.addDocument(fix.docPath, std::make_shared<DataFrame>(std::move(cols)));

    // Load (revert).
    bool loaded = fix.mgr.loadWorkspaceIfExists(fix.docPath);
    REQUIRE(loaded);
    CHECK(fix.scene.title() == "Saved Title");
}

TEST_CASE("WorkspaceManager: revertToSaved restores state",
          "[core][io][workspace_manager]") {
    ManagerFixture fix;
    fix.scene.setTitle("Original Title");

    // Register DataFrame.
    std::vector<Column> cols;
    cols.emplace_back("time", std::vector<double>{1.0, 2.0, 3.0});
    cols.emplace_back("voltage", std::vector<double>{10.0, 20.0, 30.0});
    fix.docs.addDocument(fix.docPath, std::make_shared<DataFrame>(std::move(cols)));

    // Save.
    REQUIRE(fix.mgr.saveWorkspace(fix.docPath));

    // Edit.
    fix.scene.setTitle("Edited Title");
    fix.bus.execute(std::make_unique<DummyCommand>());
    REQUIRE(fix.mgr.isModified(fix.docPath));

    // Revert.
    bool ok = fix.mgr.revertToSaved(fix.docPath);
    REQUIRE(ok);
    CHECK(fix.scene.title() == "Original Title");
    CHECK_FALSE(fix.mgr.isModified(fix.docPath));
}

TEST_CASE("WorkspaceManager: saveWorkspaceAs uses custom path",
          "[core][io][workspace_manager]") {
    ManagerFixture fix;

    const QString customPath = fix.tmpDir.path() + "/custom_workspace.lumen.json";

    bool ok = fix.mgr.saveWorkspaceAs(fix.docPath, customPath);
    REQUIRE(ok);
    CHECK(QFile::exists(customPath));

    // Default sidecar should NOT exist (only custom path was used).
    CHECK_FALSE(QFile::exists(fix.mgr.defaultSidecarPath(fix.docPath)));
}

TEST_CASE("WorkspaceManager: modifiedChanged signal not emitted when unchanged",
          "[core][io][workspace_manager]") {
    ManagerFixture fix;

    QSignalSpy modifiedSpy(&fix.mgr, &WorkspaceManager::modifiedChanged);
    REQUIRE(modifiedSpy.isValid());

    // First command sets modified from false -> true: signal emitted.
    fix.bus.execute(std::make_unique<DummyCommand>());
    CHECK(modifiedSpy.count() == 1);

    // Second command: already modified, no state change, no signal.
    fix.bus.execute(std::make_unique<DummyCommand>());
    CHECK(modifiedSpy.count() == 1);
}
