#include "MainWindow.h"

#include "DataTableDock.h"
#include "ExportDialog.h"
#include "PlotCanvas.h"
#include "PlotCanvasDock.h"

#include <core/DocumentRegistry.h>
#include <core/io/FigureExporter.h>
#include <core/io/WorkspaceManager.h>
#include <data/FileLoader.h>

#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <Qt>

namespace lumen {

namespace {
constexpr int kDefaultWidth = 1400;
constexpr int kDefaultHeight = 900;
constexpr auto kGeometryKey = "MainWindow/geometry";
constexpr auto kStateKey = "MainWindow/state";
constexpr auto kRecentFilesKey = "recentFiles";
}  // namespace

MainWindow::MainWindow(core::DocumentRegistry* registry,
                       core::PlotRegistry* plotRegistry,
                       core::CommandBus* commandBus,
                       core::io::WorkspaceManager* workspaceManager,
                       QWidget* parent)
    : QMainWindow(parent)
    , registry_(registry)
    , commandBus_(commandBus)
    , workspaceManager_(workspaceManager) {
    setWindowTitle("Lumen");
    resize(kDefaultWidth, kDefaultHeight);

    auto* placeholder = new QLabel(
        "Lumen — Phase 1\n\nOpen a CSV file via File > Open CSV...",
        this);
    placeholder->setAlignment(Qt::AlignCenter);
    setCentralWidget(placeholder);

    // Data table dock — starts hidden, shown when data is loaded
    dataTableDock_ = new ui::DataTableDock(this);
    addDockWidget(Qt::BottomDockWidgetArea, dataTableDock_);
    dataTableDock_->hide();

    // Plot canvas dock — starts hidden, shown when data is loaded
    plotCanvasDock_ = new ui::PlotCanvasDock(this);
    plotCanvasDock_->setPlotRegistry(plotRegistry);
    plotCanvasDock_->setCommandBus(commandBus);
    addDockWidget(Qt::RightDockWidgetArea, plotCanvasDock_);
    plotCanvasDock_->hide();

    // Connect workspace modification tracking.
    if (workspaceManager_ != nullptr) {
        connect(workspaceManager_, &core::io::WorkspaceManager::modifiedChanged,
                this, [this](const QString& path, bool /*modified*/) {
            if (path == currentDocPath_) {
                updateWindowTitle();
            }
        });
    }

    // Status bar
    statusBar()->showMessage(tr("Ready"));

    buildMenus();
    restoreGeometry();
}

MainWindow::~MainWindow() = default;

void MainWindow::buildMenus() {
    auto* fileMenu = menuBar()->addMenu(tr("&File"));

    // Open CSV action
    auto* openAction = fileMenu->addAction(tr("&Open CSV..."));
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    connect(openAction, &QAction::triggered, this, &MainWindow::openCsvFile);

    // Recent files submenu
    recentFilesMenu_ = fileMenu->addMenu(tr("Recent Files"));
    updateRecentFilesMenu();

    fileMenu->addSeparator();

    // Save / Revert actions
    auto* saveAction = fileMenu->addAction(tr("&Save Workspace"));
    saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveWorkspace);

    auto* saveAsAction = fileMenu->addAction(tr("Save Workspace &As..."));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::onSaveWorkspaceAs);

    auto* revertAction = fileMenu->addAction(tr("&Revert to Saved"));
    connect(revertAction, &QAction::triggered, this, &MainWindow::onRevertToSaved);

    fileMenu->addSeparator();

    auto* exportAction = fileMenu->addAction(tr("&Export Figure..."));
    exportAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportFigure);

    fileMenu->addSeparator();

    auto* quitAction = fileMenu->addAction(tr("&Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(dataTableDock_->toggleViewAction());
    viewMenu->addAction(plotCanvasDock_->toggleViewAction());

    auto* helpMenu = menuBar()->addMenu(tr("&Help"));
    auto* aboutAction = helpMenu->addAction(tr("&About Lumen"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::openCsvFile() {
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open CSV File"),
        QString(),
        tr("CSV Files (*.csv);;All Files (*)"));

    if (filePath.isEmpty()) {
        return;  // User cancelled
    }

    loadFile(filePath);
}

void MainWindow::loadFile(const QString& filePath) {
    // Check if already loaded in the registry
    const auto* existing = registry_->document(filePath);
    if (existing != nullptr) {
        dataTableDock_->showDataFrame(existing);
        dataTableDock_->show();
        plotCanvasDock_->setDataFrame(existing, filePath);
        plotCanvasDock_->show();
        currentDocPath_ = filePath;
        if (workspaceManager_ != nullptr) {
            workspaceManager_->registerPlotScene(filePath, plotCanvasDock_->scene());
        }
        updateWindowTitle();
        QFileInfo fi(filePath);
        statusBar()->showMessage(
            tr("Loaded %1 (%2 rows x %3 cols)")
                .arg(fi.fileName())
                .arg(existing->rowCount())
                .arg(existing->columnCount()));
        addRecentFile(filePath);
        return;
    }

    QFileInfo fi(filePath);
    statusBar()->showMessage(tr("Loading %1...").arg(fi.fileName()));

    // FileLoader manages its own thread. We parent it to `this` so it
    // is cleaned up if the window is destroyed during a load.
    auto* loader = new data::FileLoader(this);

    connect(loader, &data::FileLoader::finished, this,
            [this, loader](const QString& path,
                           std::shared_ptr<lumen::data::TabularBundle> bundle) {
                const auto* rawBundle = registry_->addDocument(path, std::move(bundle));

                dataTableDock_->showDataFrame(rawBundle);
                dataTableDock_->show();

                // Auto-plot.
                plotCanvasDock_->setDataFrame(rawBundle, path);
                plotCanvasDock_->show();

                currentDocPath_ = path;
                if (workspaceManager_ != nullptr) {
                    workspaceManager_->registerPlotScene(path, plotCanvasDock_->scene());
                }
                updateWindowTitle();

                QFileInfo info(path);
                statusBar()->showMessage(
                    tr("Loaded %1 (%2 rows x %3 cols)")
                        .arg(info.fileName())
                        .arg(rawBundle->rowCount())
                        .arg(rawBundle->columnCount()));

                addRecentFile(path);
                loader->deleteLater();
            });

    connect(loader, &data::FileLoader::failed, this,
            [this, loader](const QString& path, const QString& errorMessage) {
                QFileInfo info(path);
                statusBar()->showMessage(
                    tr("Failed to load %1").arg(info.fileName()));
                QMessageBox::critical(
                    this,
                    tr("Load Error"),
                    tr("Failed to load %1:\n%2")
                        .arg(info.fileName(), errorMessage));
                loader->deleteLater();
            });

    loader->load(filePath);
}

void MainWindow::addRecentFile(const QString& filePath) {
    QSettings settings;
    QStringList recent = settings.value(kRecentFilesKey).toStringList();

    // Remove duplicates of this path, then prepend
    recent.removeAll(filePath);
    recent.prepend(filePath);

    // Trim to max
    while (recent.size() > kMaxRecentFiles) {
        recent.removeLast();
    }

    settings.setValue(kRecentFilesKey, recent);
    updateRecentFilesMenu();
}

void MainWindow::updateRecentFilesMenu() {
    recentFilesMenu_->clear();

    QSettings settings;
    const QStringList recent =
        settings.value(kRecentFilesKey).toStringList();

    if (recent.isEmpty()) {
        auto* emptyAction = recentFilesMenu_->addAction(tr("(No recent files)"));
        emptyAction->setEnabled(false);
        return;
    }

    for (const QString& path : recent) {
        QFileInfo fi(path);
        auto* action = recentFilesMenu_->addAction(fi.fileName());
        action->setToolTip(path);
        action->setData(path);
        connect(action, &QAction::triggered, this, [this, path]() {
            loadFile(path);
        });
    }

    recentFilesMenu_->addSeparator();
    auto* clearAction = recentFilesMenu_->addAction(tr("Clear Recent Files"));
    connect(clearAction, &QAction::triggered, this, [this]() {
        QSettings s;
        s.remove(kRecentFilesKey);
        updateRecentFilesMenu();
    });
}

void MainWindow::restoreGeometry() {
    QSettings settings;
    const auto geometry = settings.value(kGeometryKey).toByteArray();
    if (!geometry.isEmpty()) {
        QMainWindow::restoreGeometry(geometry);
    }
    const auto state = settings.value(kStateKey).toByteArray();
    if (!state.isEmpty()) {
        QMainWindow::restoreState(state);
    }
}

void MainWindow::saveGeometry() const {
    QSettings settings;
    settings.setValue(kGeometryKey, QMainWindow::saveGeometry());
    settings.setValue(kStateKey, QMainWindow::saveState());
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (!currentDocPath_.isEmpty() && workspaceManager_ != nullptr &&
        workspaceManager_->isModified(currentDocPath_)) {
        auto result = QMessageBox::question(
            this, tr("Unsaved Changes"),
            tr("Save workspace for %1?").arg(QFileInfo(currentDocPath_).fileName()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (result == QMessageBox::Save) {
            workspaceManager_->saveWorkspace(currentDocPath_);
        } else if (result == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    saveGeometry();
    QMainWindow::closeEvent(event);
}

void MainWindow::showAbout() {
    QMessageBox::about(
        this,
        tr("About Lumen"),
        tr("<h3>Lumen %1</h3>"
           "<p>Standalone interactive scientific plot viewer.</p>"
           "<p>Phase 1 — Data Layer and First UI Shell.</p>")
            .arg(QApplication::applicationVersion()));
}

void MainWindow::onSaveWorkspace() {
    if (currentDocPath_.isEmpty() || workspaceManager_ == nullptr) {
        return;
    }
    if (workspaceManager_->saveWorkspace(currentDocPath_)) {
        statusBar()->showMessage(tr("Workspace saved"), 3000);
    } else {
        statusBar()->showMessage(tr("Failed to save workspace"), 3000);
    }
}

void MainWindow::onSaveWorkspaceAs() {
    if (currentDocPath_.isEmpty() || workspaceManager_ == nullptr) {
        return;
    }
    QString path = QFileDialog::getSaveFileName(
        this, tr("Save Workspace As"),
        workspaceManager_->defaultSidecarPath(currentDocPath_),
        tr("Lumen Workspace (*.lumen.json)"));
    if (!path.isEmpty()) {
        if (workspaceManager_->saveWorkspaceAs(currentDocPath_, path)) {
            statusBar()->showMessage(
                tr("Workspace saved to %1").arg(path), 3000);
        } else {
            statusBar()->showMessage(tr("Failed to save workspace"), 3000);
        }
    }
}

void MainWindow::onRevertToSaved() {
    if (currentDocPath_.isEmpty() || workspaceManager_ == nullptr) {
        return;
    }
    if (workspaceManager_->revertToSaved(currentDocPath_)) {
        plotCanvasDock_->canvas()->update();
        statusBar()->showMessage(tr("Reverted to saved workspace"), 3000);
    }
}

void MainWindow::onExportFigure() {
    if (plotCanvasDock_ == nullptr || plotCanvasDock_->scene() == nullptr) {
        return;
    }

    ui::ExportDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    auto opts = dialog.options();

    core::io::FigureExporter::Options exportOpts;
    exportOpts.format = static_cast<core::io::FigureExporter::Format>(opts.format);
    exportOpts.widthPx = opts.widthPx;
    exportOpts.heightPx = opts.heightPx;
    exportOpts.dpi = opts.dpi;
    exportOpts.transparentBackground = opts.transparentBackground;
    exportOpts.outputPath = opts.outputPath;

    QString error = core::io::FigureExporter::exportFigure(
        plotCanvasDock_->scene(), exportOpts);

    if (error.isEmpty()) {
        statusBar()->showMessage(
            tr("Exported to %1").arg(opts.outputPath), 3000);
    } else {
        QMessageBox::warning(this, tr("Export Failed"), error);
    }
}

void MainWindow::updateWindowTitle() {
    QString title = QStringLiteral("Lumen");
    if (!currentDocPath_.isEmpty()) {
        QFileInfo fi(currentDocPath_);
        title = QStringLiteral("Lumen — %1").arg(fi.fileName());
        if (workspaceManager_ != nullptr &&
            workspaceManager_->isModified(currentDocPath_)) {
            title += QStringLiteral(" \u25CF");
        }
    }
    setWindowTitle(title);
}

}  // namespace lumen
