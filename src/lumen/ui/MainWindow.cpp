#include "MainWindow.h"

#include "DataTableDock.h"
#include "PlotCanvasDock.h"

#include <core/DocumentRegistry.h>
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
                       QWidget* parent)
    : QMainWindow(parent)
    , registry_(registry)
    , commandBus_(commandBus) {
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
        QFileInfo fi(filePath);
        setWindowTitle(QStringLiteral("Lumen — %1").arg(fi.fileName()));
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
                           std::shared_ptr<lumen::data::DataFrame> df) {
                const auto* rawDf = registry_->addDocument(path, std::move(df));

                dataTableDock_->showDataFrame(rawDf);
                dataTableDock_->show();

                // Auto-plot.
                plotCanvasDock_->setDataFrame(rawDf, path);
                plotCanvasDock_->show();

                QFileInfo info(path);
                setWindowTitle(
                    QStringLiteral("Lumen — %1").arg(info.fileName()));
                statusBar()->showMessage(
                    tr("Loaded %1 (%2 rows x %3 cols)")
                        .arg(info.fileName())
                        .arg(rawDf->rowCount())
                        .arg(rawDf->columnCount()));

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

}  // namespace lumen
