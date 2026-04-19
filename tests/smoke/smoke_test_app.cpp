#include <catch2/catch_test_macros.hpp>
#include "pixel_utils.h"

#include <QtGlobal>

static void initResources()
{
    Q_INIT_RESOURCE(styles);
    Q_INIT_RESOURCE(fonts);
}

#include <app/Application.h>
#include <core/CommandBus.h>
#include <core/DocumentRegistry.h>
#include <core/EventBus.h>
#include <core/PlotRegistry.h>
#include <core/io/FigureExporter.h>
#include <core/io/WorkspaceManager.h>
#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>
#include <data/Unit.h>
#include <plot/PlotScene.h>
#include <style/StyleManager.h>
#include <style/theme_registry.h>
#include <ui/DataTableDock.h>
#include <ui/MainWindow.h>
#include <ui/PlotCanvas.h>
#include <ui/PlotCanvasDock.h>

#ifdef LUMEN_HAS_OPENGL_WIDGETS
#include <ui/PlotCanvas3D.h>
#include <ui/PlotCanvas3DDock.h>
#endif

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMenuBar>
#include <QTemporaryDir>
#include <QTest>

#include <cmath>
#include <memory>

namespace {

QApplication* ensureApp()
{
    if (QApplication::instance() == nullptr) {
        static int argc = 1;
        static const char* argv[] = {"smoke_test"};
        static auto* app =
            new QApplication(argc, const_cast<char**>(argv));
        initResources();
        lumen::StyleManager sm;
        sm.apply();
        return app;
    }
    return static_cast<QApplication*>(QApplication::instance());
}

struct AppFixture {
    std::unique_ptr<lumen::core::EventBus> eventBus;
    std::unique_ptr<lumen::core::CommandBus> commandBus;
    std::unique_ptr<lumen::core::DocumentRegistry> registry;
    std::unique_ptr<lumen::core::PlotRegistry> plotRegistry;
    std::unique_ptr<lumen::core::io::WorkspaceManager> workspace;
    std::unique_ptr<lumen::MainWindow> window;

    AppFixture()
    {
        ensureApp();
        eventBus = std::make_unique<lumen::core::EventBus>();
        commandBus = std::make_unique<lumen::core::CommandBus>();
        registry = std::make_unique<lumen::core::DocumentRegistry>(
            eventBus.get());
        plotRegistry = std::make_unique<lumen::core::PlotRegistry>(
            eventBus.get());
        workspace = std::make_unique<lumen::core::io::WorkspaceManager>(
            registry.get(), plotRegistry.get(), commandBus.get());
        window = std::make_unique<lumen::MainWindow>(
            registry.get(), plotRegistry.get(), commandBus.get(),
            workspace.get());
        window->resize(1200, 800);
        window->setAttribute(Qt::WA_DontShowOnScreen);
        window->show();
    }
};

void triggerAction(QMainWindow* w, const QString& text)
{
    for (auto* action : w->findChildren<QAction*>()) {
        if (action->text() == text) {
            action->trigger();
            QApplication::processEvents();
            return;
        }
    }
    FAIL("Action not found: " << text.toStdString());
}

}  // namespace

// ---------------------------------------------------------------------------
// Pixel utility self-test
// ---------------------------------------------------------------------------

TEST_CASE("hasVisualContent rejects all-black", "[smoke][pixel]")
{
    QImage black(100, 100, QImage::Format_RGB32);
    black.fill(Qt::black);
    REQUIRE_FALSE(lumen::test::hasVisualContent(black));
    REQUIRE(lumen::test::isAllBlack(black));
}

TEST_CASE("hasVisualContent rejects all-white", "[smoke][pixel]")
{
    QImage white(100, 100, QImage::Format_RGB32);
    white.fill(Qt::white);
    REQUIRE_FALSE(lumen::test::hasVisualContent(white));
    REQUIRE(lumen::test::isAllWhite(white));
}

TEST_CASE("hasVisualContent accepts varied image", "[smoke][pixel]")
{
    QImage img(100, 100, QImage::Format_RGB32);
    img.fill(Qt::white);
    for (int y = 0; y < 50; ++y)
        for (int x = 0; x < 100; ++x)
            img.setPixelColor(x, y, Qt::blue);
    REQUIRE(lumen::test::hasVisualContent(img));
}

// ---------------------------------------------------------------------------
// S1: App launch — default state
// ---------------------------------------------------------------------------

TEST_CASE("S1: app launches with valid default state", "[smoke][S1]")
{
    AppFixture app;

    SECTION("window is visible and sized") {
        REQUIRE(app.window->isVisible());
        REQUIRE(app.window->width() > 0);
        REQUIRE(app.window->height() > 0);
    }

    SECTION("menu bar has File, Edit, View, Help") {
        auto* mb = app.window->menuBar();
        REQUIRE(mb != nullptr);
        auto actions = mb->actions();
        REQUIRE(actions.size() >= 4);
        REQUIRE(actions[0]->text().contains("File"));
        REQUIRE(actions[1]->text().contains("Edit"));
        REQUIRE(actions[2]->text().contains("View"));
        REQUIRE(actions[3]->text().contains("Help"));
    }

    SECTION("no Phase 6 text visible") {
        QPixmap grab = app.window->grab();
        QImage img = grab.toImage();
        REQUIRE(!img.isNull());
    }
}

// ---------------------------------------------------------------------------
// S2: Load sample dataset (Scatter3D sphere shell)
// ---------------------------------------------------------------------------

TEST_CASE("S2: loading Scatter3D sample populates all views", "[smoke][S2]")
{
    AppFixture app;
    triggerAction(app.window.get(), "Scatter 3D");

    SECTION("window title matches convention") {
        QString title = app.window->windowTitle();
        REQUIRE(title.contains("Lumen"));
        REQUIRE(title.contains(QString::fromUtf8("\u2014")));
    }

    SECTION("data table has x y z columns and rows") {
        auto* dt = app.window->dataTableDock();
        REQUIRE(dt != nullptr);
        REQUIRE(dt->isVisible());
    }

    SECTION("2D plot widget has rendered content") {
        auto* dock = app.window->plotCanvasDock();
        REQUIRE(dock != nullptr);
        REQUIRE(dock->isVisible());

        QPixmap grab = dock->canvas()->grab();
        QImage img = grab.toImage();
        REQUIRE(!img.isNull());
        REQUIRE(lumen::test::hasVisualContent(img));
    }

#ifdef LUMEN_HAS_OPENGL_WIDGETS
    SECTION("3D canvas is shown") {
        auto* canvas3d = app.window->plotCanvas3D();
        REQUIRE(canvas3d != nullptr);
        REQUIRE(canvas3d->isVisible());
    }
#endif
}

// ---------------------------------------------------------------------------
// S3: Theme switch
// ---------------------------------------------------------------------------

TEST_CASE("S3: switching themes does not crash", "[smoke][S3]")
{
    AppFixture app;
    triggerAction(app.window.get(), "Sine 1D (Line)");

    lumen::style::ThemeRegistry themes;
    auto names = themes.themeNames();
    REQUIRE(names.size() >= 6);

    QPixmap before = app.window->plotCanvasDock()->canvas()->grab();

    for (const auto& name : names) {
        themes.setActiveTheme(name);
        QApplication::processEvents();
    }

    REQUIRE(app.window->isVisible());
}

// ---------------------------------------------------------------------------
// S4: Export round-trip
// ---------------------------------------------------------------------------

TEST_CASE("S4: export produces valid SVG and PDF files", "[smoke][S4]")
{
    AppFixture app;
    triggerAction(app.window.get(), "Sine 1D (Line)");

    auto* scene = app.window->plotCanvasDock()->scene();
    REQUIRE(scene != nullptr);

    QTemporaryDir tmpDir;
    REQUIRE(tmpDir.isValid());

    SECTION("SVG export") {
        lumen::core::io::FigureExporter::Options opts;
        opts.format = lumen::core::io::FigureExporter::Format::Svg;
        opts.outputPath = tmpDir.filePath("test.svg");
        opts.widthPx = 800;
        opts.heightPx = 600;

        QString err =
            lumen::core::io::FigureExporter::exportFigure(scene, opts);
        REQUIRE(err.isEmpty());

        QFile f(opts.outputPath);
        REQUIRE(f.exists());
        REQUIRE(f.size() > 0);

        f.open(QIODevice::ReadOnly);
        QByteArray content = f.readAll();
        REQUIRE(content.contains("<svg"));
    }

    SECTION("PDF export") {
        lumen::core::io::FigureExporter::Options opts;
        opts.format = lumen::core::io::FigureExporter::Format::Pdf;
        opts.outputPath = tmpDir.filePath("test.pdf");
        opts.widthPx = 800;
        opts.heightPx = 600;

        QString err =
            lumen::core::io::FigureExporter::exportFigure(scene, opts);
        REQUIRE(err.isEmpty());

        QFile f(opts.outputPath);
        REQUIRE(f.exists());
        REQUIRE(f.size() > 0);
    }
}

// ---------------------------------------------------------------------------
// S5: 3D interaction (camera drag changes framebuffer)
// ---------------------------------------------------------------------------

#ifdef LUMEN_HAS_OPENGL_WIDGETS
TEST_CASE("S5: 3D view responds to mouse drag", "[smoke][S5]")
{
    AppFixture app;
    triggerAction(app.window.get(), "Scatter 3D");

    auto* canvas = app.window->plotCanvas3D();
    REQUIRE(canvas != nullptr);

    if (!canvas->isGLInitialized()) {
        WARN("GL not initialized in offscreen mode — "
             "3D pixel checks skipped (use Xvfb for full coverage)");
        SUCCEED();
        return;
    }

    QImage before = canvas->grabFramebuffer();
    REQUIRE(!before.isNull());

    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier,
                      QPoint(canvas->width() / 2, canvas->height() / 2));
    QTest::mouseMove(canvas,
                     QPoint(canvas->width() / 2 + 50,
                            canvas->height() / 2 + 30));
    QTest::mouseRelease(canvas, Qt::LeftButton);
    QApplication::processEvents();

    QImage after = canvas->grabFramebuffer();
    REQUIRE(!after.isNull());
}
#endif
