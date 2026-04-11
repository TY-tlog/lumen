#include <catch2/catch_test_macros.hpp>

#include "core/CommandBus.h"
#include "core/DocumentRegistry.h"
#include "core/PlotRegistry.h"
#include "core/io/WorkspaceFile.h"
#include "core/io/WorkspaceManager.h"
#include "data/Rank1Dataset.h"
#include "data/TabularBundle.h"
#include "data/Unit.h"
#include "plot/LineSeries.h"
#include "plot/PlotScene.h"
#include "plot/PlotStyle.h"

#include <QSignalSpy>
#include <QTemporaryDir>

#include <memory>

using lumen::core::CommandBus;
using lumen::core::DocumentRegistry;
using lumen::core::PlotRegistry;
using lumen::core::io::WorkspaceFile;
using lumen::core::io::WorkspaceManager;
using lumen::data::Rank1Dataset;
using lumen::data::TabularBundle;
using lumen::data::Unit;
using lumen::plot::LineSeries;
using lumen::plot::PlotScene;
using lumen::plot::PlotStyle;

namespace {

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
    std::shared_ptr<Rank1Dataset> xDs = std::make_shared<Rank1Dataset>("time", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0});
    std::shared_ptr<Rank1Dataset> yDs = std::make_shared<Rank1Dataset>("voltage", Unit::dimensionless(), std::vector<double>{10.0, 20.0, 30.0});

    ManagerFixture()
    {
        REQUIRE(tmpDir.isValid());
        docPath = tmpDir.path() + "/data.csv";

        QFile f(docPath);
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write("time,voltage\n1,10\n2,20\n3,30\n");
        f.close();

        PlotStyle style;
        style.color = QColor(Qt::red);
        scene.addSeries(LineSeries(xDs, yDs, style, "Voltage"));
        scene.setTitle("My Plot");

        mgr.registerPlotScene(docPath, &scene);
    }
};

}  // namespace

TEST_CASE("WorkspaceManager: modified flag set on commandExecuted", "[core][io][workspace_manager]") {
    ManagerFixture fix;
    CHECK_FALSE(fix.mgr.isModified(fix.docPath));
    fix.bus.execute(std::make_unique<DummyCommand>());
    CHECK(fix.mgr.isModified(fix.docPath));
}

TEST_CASE("WorkspaceManager: modified flag cleared on save", "[core][io][workspace_manager]") {
    ManagerFixture fix;
    fix.bus.execute(std::make_unique<DummyCommand>());
    REQUIRE(fix.mgr.isModified(fix.docPath));
    bool ok = fix.mgr.saveWorkspace(fix.docPath);
    CHECK(ok);
    CHECK_FALSE(fix.mgr.isModified(fix.docPath));
}

TEST_CASE("WorkspaceManager: defaultSidecarPath correct", "[core][io][workspace_manager]") {
    ManagerFixture fix;
    CHECK(fix.mgr.defaultSidecarPath("/path/to/data.csv") == "/path/to/data.lumen.json");
    CHECK(fix.mgr.defaultSidecarPath("/path/to/data.CSV") == "/path/to/data.lumen.json");
    CHECK(fix.mgr.defaultSidecarPath("/path/to/data.tsv") == "/path/to/data.tsv.lumen.json");
    CHECK(fix.mgr.defaultSidecarPath("/path/to/myfile") == "/path/to/myfile.lumen.json");
}

TEST_CASE("WorkspaceManager: loadWorkspaceIfExists no-op without sidecar", "[core][io][workspace_manager]") {
    ManagerFixture fix;
    bool loaded = fix.mgr.loadWorkspaceIfExists(fix.docPath);
    CHECK_FALSE(loaded);
}

TEST_CASE("WorkspaceManager: signals emitted on save", "[core][io][workspace_manager]") {
    ManagerFixture fix;
    QSignalSpy savedSpy(&fix.mgr, &WorkspaceManager::workspaceSaved);
    QSignalSpy modifiedSpy(&fix.mgr, &WorkspaceManager::modifiedChanged);
    REQUIRE(savedSpy.isValid());
    REQUIRE(modifiedSpy.isValid());
    fix.bus.execute(std::make_unique<DummyCommand>());
    REQUIRE(modifiedSpy.count() == 1);
    CHECK(modifiedSpy.at(0).at(1).toBool() == true);
    fix.mgr.saveWorkspace(fix.docPath);
    CHECK(savedSpy.count() == 1);
    CHECK(savedSpy.at(0).at(0).toString() == fix.docPath);
    CHECK(modifiedSpy.count() == 2);
    CHECK(modifiedSpy.at(1).at(1).toBool() == false);
}

TEST_CASE("WorkspaceManager: save then load roundtrip", "[core][io][workspace_manager]") {
    ManagerFixture fix;
    fix.scene.setTitle("Saved Title");
    bool ok = fix.mgr.saveWorkspace(fix.docPath);
    REQUIRE(ok);
    const QString sidecar = fix.mgr.defaultSidecarPath(fix.docPath);
    CHECK(QFile::exists(sidecar));
    fix.scene.setTitle("Changed Title");

    // Register TabularBundle for column resolution on load.
    auto bundle = std::make_shared<TabularBundle>();
    bundle->addColumn(std::make_shared<Rank1Dataset>("time", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0}));
    bundle->addColumn(std::make_shared<Rank1Dataset>("voltage", Unit::dimensionless(), std::vector<double>{10.0, 20.0, 30.0}));
    fix.docs.addDocument(fix.docPath, bundle);

    bool loaded = fix.mgr.loadWorkspaceIfExists(fix.docPath);
    REQUIRE(loaded);
    CHECK(fix.scene.title() == "Saved Title");
}

TEST_CASE("WorkspaceManager: revertToSaved restores state", "[core][io][workspace_manager]") {
    ManagerFixture fix;
    fix.scene.setTitle("Original Title");

    auto bundle = std::make_shared<TabularBundle>();
    bundle->addColumn(std::make_shared<Rank1Dataset>("time", Unit::dimensionless(), std::vector<double>{1.0, 2.0, 3.0}));
    bundle->addColumn(std::make_shared<Rank1Dataset>("voltage", Unit::dimensionless(), std::vector<double>{10.0, 20.0, 30.0}));
    fix.docs.addDocument(fix.docPath, bundle);

    REQUIRE(fix.mgr.saveWorkspace(fix.docPath));
    fix.scene.setTitle("Edited Title");
    fix.bus.execute(std::make_unique<DummyCommand>());
    REQUIRE(fix.mgr.isModified(fix.docPath));
    bool ok = fix.mgr.revertToSaved(fix.docPath);
    REQUIRE(ok);
    CHECK(fix.scene.title() == "Original Title");
    CHECK_FALSE(fix.mgr.isModified(fix.docPath));
}

TEST_CASE("WorkspaceManager: saveWorkspaceAs uses custom path", "[core][io][workspace_manager]") {
    ManagerFixture fix;
    const QString customPath = fix.tmpDir.path() + "/custom_workspace.lumen.json";
    bool ok = fix.mgr.saveWorkspaceAs(fix.docPath, customPath);
    REQUIRE(ok);
    CHECK(QFile::exists(customPath));
    CHECK_FALSE(QFile::exists(fix.mgr.defaultSidecarPath(fix.docPath)));
}

TEST_CASE("WorkspaceManager: modifiedChanged signal not emitted when unchanged", "[core][io][workspace_manager]") {
    ManagerFixture fix;
    QSignalSpy modifiedSpy(&fix.mgr, &WorkspaceManager::modifiedChanged);
    REQUIRE(modifiedSpy.isValid());
    fix.bus.execute(std::make_unique<DummyCommand>());
    CHECK(modifiedSpy.count() == 1);
    fix.bus.execute(std::make_unique<DummyCommand>());
    CHECK(modifiedSpy.count() == 1);
}
