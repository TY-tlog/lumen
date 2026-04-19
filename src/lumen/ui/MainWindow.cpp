#include "MainWindow.h"

#include "DataTableDock.h"
#include "ExportDialog.h"
#include "PlotCanvas.h"
#include "PlotCanvas3D.h"
#include "PlotCanvas3DDock.h"
#include "PlotCanvasDock.h"

#include <core/DocumentRegistry.h>
#include <core/io/FigureExporter.h>
#include <core/io/WorkspaceManager.h>
#include <data/CoordinateArray.h>
#include <data/Dimension.h>
#include <data/FileLoader.h>
#include <data/Grid2D.h>
#include <data/MemoryManager.h>
#include <data/Rank1Dataset.h>
#include <data/TabularBundle.h>
#include <data/Unit.h>
#include <data/Volume3D.h>
#include <data/io/DatasetLoader.h>
#include <data/io/LoaderRegistry.h>
#include <plot/Colormap.h>
#include <plot/Heatmap.h>
#include <plot/PlotScene.h>
#include <plot3d/Scatter3D.h>
#include <plot3d/Surface3D.h>
#include <plot3d/VolumeItem.h>

#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QTimer>
#include <Qt>

#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>

namespace lumen {

namespace {
constexpr int kDefaultWidth = 1400;
constexpr int kDefaultHeight = 900;
constexpr auto kGeometryKey = "MainWindow/geometry";
constexpr auto kStateKey = "MainWindow/state";
constexpr auto kRecentFilesKey = "recentFiles";
constexpr int kMemoryUpdateIntervalMs = 2000;
constexpr double kOneGB = 1024.0 * 1024.0 * 1024.0;
constexpr double kOneMB = 1024.0 * 1024.0;

QString formatBytes(std::size_t bytes) {
    auto value = static_cast<double>(bytes);
    if (value >= kOneGB) {
        return QStringLiteral("%1 GB").arg(value / kOneGB, 0, 'f', 1);
    }
    return QStringLiteral("%1 MB").arg(
        static_cast<int>(std::round(value / kOneMB)));
}
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

    // Outer: horizontal splitter [2D content | 3D view]
    hSplitter_ = new QSplitter(Qt::Horizontal, this);
    hSplitter_->setHandleWidth(6);
    hSplitter_->setChildrenCollapsible(false);

    // Inner: vertical splitter [plot (70%) | data table (30%)]
    vSplitter_ = new QSplitter(Qt::Vertical);
    vSplitter_->setHandleWidth(6);
    vSplitter_->setChildrenCollapsible(false);

    // Placeholder shown until data loads.
    placeholder_ = new QLabel(
        QStringLiteral("Open a file via File \u203A Open, or choose a sample from the Samples menu."));
    placeholder_->setAlignment(Qt::AlignCenter);
    placeholder_->setStyleSheet(QStringLiteral("QLabel { color: #86868b; font-size: 14px; }"));

    plotCanvasDock_ = new ui::PlotCanvasDock(vSplitter_);
    plotCanvasDock_->setPlotRegistry(plotRegistry);
    plotCanvasDock_->setCommandBus(commandBus);
    plotCanvasDock_->setTitleBarWidget(new QWidget());
    plotCanvasDock_->setFeatures(QDockWidget::NoDockWidgetFeatures);
    plotCanvasDock_->hide();

    dataTableDock_ = new ui::DataTableDock(vSplitter_);
    dataTableDock_->setFeatures(QDockWidget::NoDockWidgetFeatures);
    dataTableDock_->hide();

    vSplitter_->addWidget(placeholder_);
    vSplitter_->addWidget(plotCanvasDock_);
    vSplitter_->addWidget(dataTableDock_);
    vSplitter_->setStretchFactor(0, 1);
    vSplitter_->setStretchFactor(1, 3);
    vSplitter_->setStretchFactor(2, 1);

    // 3D canvas directly in the horizontal splitter.
    plotCanvas3D_ = new ui::PlotCanvas3D(hSplitter_);
    plotCanvas3D_->setMinimumSize(0, 0);
    plotCanvas3D_->setMaximumSize(0, 0);

    plotCanvas3DDock_ = nullptr;

    hSplitter_->addWidget(vSplitter_);
    hSplitter_->addWidget(plotCanvas3D_);
    hSplitter_->setStretchFactor(0, 1);
    hSplitter_->setStretchFactor(1, 0);
    hSplitter_->setCollapsible(1, true);
    plotCanvas3D_->hide();
    hSplitter_->setSizes({1, 0});

    setCentralWidget(hSplitter_);

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
    setupMemoryStatusBar();
    restoreGeometry();
}

MainWindow::~MainWindow() = default;

void MainWindow::buildMenus() {
    auto* fileMenu = menuBar()->addMenu(tr("&File"));

    // Open action (universal — all registered formats)
    auto* openAction = fileMenu->addAction(tr("&Open..."));
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    // Recent files submenu
    recentFilesMenu_ = fileMenu->addMenu(tr("Recent Files"));
    updateRecentFilesMenu();

    // Sample data submenu
    buildSampleMenu(fileMenu);

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

    // Edit menu with Preferences
    auto* editMenu = menuBar()->addMenu(tr("&Edit"));
    auto* prefsAction = editMenu->addAction(tr("&Preferences..."));
    connect(prefsAction, &QAction::triggered, this, &MainWindow::showMemoryBudgetDialog);

    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    auto* toggleData = viewMenu->addAction(tr("Data Table"));
    toggleData->setCheckable(true);
    toggleData->setChecked(true);
    connect(toggleData, &QAction::toggled, dataTableDock_, &QWidget::setVisible);

    auto* togglePlot = viewMenu->addAction(tr("Plot"));
    togglePlot->setCheckable(true);
    togglePlot->setChecked(true);
    connect(togglePlot, &QAction::toggled, plotCanvasDock_, &QWidget::setVisible);

    auto* toggle3D = viewMenu->addAction(tr("3D View"));
    toggle3D->setCheckable(true);
    connect(toggle3D, &QAction::toggled, this, [this](bool checked) {
        if (checked) {
            plotCanvas3D_->setMinimumSize(200, 200);
            plotCanvas3D_->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            plotCanvas3D_->setVisible(true);
            QList<int> sizes = hSplitter_->sizes();
            int total = sizes[0] + sizes[1];
            sizes[0] = total * 2 / 3;
            sizes[1] = total / 3;
            hSplitter_->setSizes(sizes);
        } else {
            plotCanvas3D_->setVisible(false);
            plotCanvas3D_->setMinimumSize(0, 0);
            QList<int> sizes = hSplitter_->sizes();
            sizes[1] = 0;
            hSplitter_->setSizes(sizes);
        }
    });

    viewMenu->addSeparator();
    auto* darkModeAction = viewMenu->addAction(tr("Dark Mode"));
    darkModeAction->setCheckable(true);
    connect(darkModeAction, &QAction::toggled, this, [](bool checked) {
        QString path = checked ? QStringLiteral(":/styles/dark.qss")
                               : QStringLiteral(":/styles/light.qss");
        QFile f(path);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qApp->setStyleSheet(f.readAll());
        }
    });

    auto* helpMenu = menuBar()->addMenu(tr("&Help"));
    auto* aboutAction = helpMenu->addAction(tr("&About Lumen"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::buildSampleMenu(QMenu* fileMenu) {
    auto* sampleMenu = fileMenu->addMenu(tr("Open Sample"));

    // 2D plot samples
    auto* sub2d = sampleMenu->addMenu(tr("2D Plots"));
    connect(sub2d->addAction(tr("Sine 1D (Line)")), &QAction::triggered, this, &MainWindow::openSampleSine1D);
    connect(sub2d->addAction(tr("Scatter 2D")), &QAction::triggered, this, &MainWindow::openSampleScatter2D);
    connect(sub2d->addAction(tr("Bar Chart")), &QAction::triggered, this, &MainWindow::openSampleBarChart);
    connect(sub2d->addAction(tr("Histogram")), &QAction::triggered, this, &MainWindow::openSampleHistogram);
    connect(sub2d->addAction(tr("Box Plot")), &QAction::triggered, this, &MainWindow::openSampleBoxPlot);
    connect(sub2d->addAction(tr("Violin Plot")), &QAction::triggered, this, &MainWindow::openSampleViolin);
    connect(sub2d->addAction(tr("Gaussian 2D (Heatmap)")), &QAction::triggered, this, &MainWindow::openSampleGaussian2D);
    connect(sub2d->addAction(tr("Mandelbrot (Heatmap)")), &QAction::triggered, this, &MainWindow::openSampleMandelbrot);

    // 3D plot samples
    auto* sub3d = sampleMenu->addMenu(tr("3D Plots"));
    connect(sub3d->addAction(tr("Scatter 3D")), &QAction::triggered, this, &MainWindow::openSampleScatter3D);
    connect(sub3d->addAction(tr("Surface 3D")), &QAction::triggered, this, &MainWindow::openSampleSurface3D);
    connect(sub3d->addAction(tr("Volume Sphere")), &QAction::triggered, this, &MainWindow::openSampleVolumeSphere);
    connect(sub3d->addAction(tr("Streamlines")), &QAction::triggered, this, &MainWindow::openSampleStreamlines);
    connect(sub3d->addAction(tr("Isosurface")), &QAction::triggered, this, &MainWindow::openSampleIsosurface);
}

void MainWindow::setupMemoryStatusBar() {
    memoryLabel_ = new QLabel(this);
    memoryLabel_->setObjectName(QStringLiteral("MemoryStatusLabel"));
    statusBar()->addPermanentWidget(memoryLabel_);

    memoryTimer_ = new QTimer(this);
    connect(memoryTimer_, &QTimer::timeout, this, &MainWindow::updateMemoryStatus);
    memoryTimer_->start(kMemoryUpdateIntervalMs);

    // Initial update
    updateMemoryStatus();
}

void MainWindow::updateMemoryStatus() {
    const auto& mm = data::MemoryManager::instance();
    const auto used = mm.currentUsageBytes();
    const auto budget = mm.memoryBudgetBytes();
    memoryLabel_->setText(
        QStringLiteral("Mem: %1 / %2").arg(formatBytes(used), formatBytes(budget)));
}

void MainWindow::openFile() {
    // Use LoaderRegistry for file filter
    QString filter = data::io::LoaderRegistry::instance().fileFilter();
    if (filter.isEmpty()) {
        filter = tr("All Files (*)");
    } else {
        filter += QStringLiteral(";;") + tr("All Files (*)");
    }

    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        QString(),
        filter);

    if (filePath.isEmpty()) {
        return;  // User cancelled
    }

    loadFile(filePath);
}

void MainWindow::loadFile(const QString& filePath) {
    plotCanvas3D_->setVisible(false);
    plotCanvas3D_->setMinimumSize(0, 0);
    {
        QList<int> sz = hSplitter_->sizes();
        sz[1] = 0;
        hSplitter_->setSizes(sz);
    }

    // Check if already loaded in the registry (tabular)
    const auto* existing = registry_->document(filePath);
    if (existing != nullptr) {
        showTabular(existing, filePath);
        addRecentFile(filePath);
        return;
    }

    QFileInfo fi(filePath);
    statusBar()->showMessage(tr("Loading %1...").arg(fi.fileName()));

    // Try LoaderRegistry first for all formats
    auto* registryLoader = data::io::LoaderRegistry::instance().loaderForPath(filePath);
    if (registryLoader != nullptr) {
        // Try tabular first
        auto tabular = registryLoader->loadTabular(filePath);
        if (tabular) {
            const auto* rawBundle = registry_->addDocument(filePath, std::move(tabular));
            showTabular(rawBundle, filePath);
            addRecentFile(filePath);
            return;
        }
        // Try dataset (Grid2D/Volume3D)
        auto dataset = registryLoader->loadDataset(filePath);
        if (dataset) {
            showDataset(dataset.get(), fi.fileName());
            statusBar()->showMessage(
                tr("Loaded %1 (dataset, rank %2)")
                    .arg(fi.fileName())
                    .arg(dataset->rank()));
            addRecentFile(filePath);
            return;
        }
    }

    // Fallback: use FileLoader (async CSV loader) for .csv files
    auto* loader = new data::FileLoader(this);

    connect(loader, &data::FileLoader::finished, this,
            [this, loader](const QString& path,
                           std::shared_ptr<lumen::data::TabularBundle> bundle) {
                const auto* rawBundle = registry_->addDocument(path, std::move(bundle));
                showTabular(rawBundle, path);
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

void MainWindow::showTabular(const data::TabularBundle* bundle, const QString& path) {
    if (placeholder_ != nullptr) {
        placeholder_->hide();
        placeholder_ = nullptr;
    }

    dataTableDock_->showDataFrame(bundle);
    dataTableDock_->show();

    plotCanvasDock_->setDataFrame(bundle, path);
    plotCanvasDock_->show();

    vSplitter_->setStretchFactor(1, 3);
    vSplitter_->setStretchFactor(2, 1);

    currentDocPath_ = path;
    if (workspaceManager_ != nullptr) {
        workspaceManager_->registerPlotScene(path, plotCanvasDock_->scene());
    }
    updateWindowTitle();

    QFileInfo info(path);
    statusBar()->showMessage(
        tr("Loaded %1 (%2 rows x %3 cols)")
            .arg(info.fileName())
            .arg(bundle->rowCount())
            .arg(bundle->columnCount()));
}

void MainWindow::showDataset(const data::Dataset* ds, const QString& label) {
    if (placeholder_ != nullptr) {
        placeholder_->hide();
        placeholder_ = nullptr;
    }
    dataTableDock_->showDatasetInfo(ds);
    dataTableDock_->show();

    // If Grid2D, create a Heatmap and show in PlotCanvasDock.
    if (ds->rank() == 2) {
        auto* grid = dynamic_cast<const data::Grid2D*>(ds);
        if (grid) {
            // Find the shared_ptr in sampleDatasets_ to pass to Heatmap.
            std::shared_ptr<data::Grid2D> gridPtr;
            for (const auto& sample : sampleDatasets_) {
                if (sample.get() == ds) {
                    gridPtr = std::dynamic_pointer_cast<data::Grid2D>(sample);
                    break;
                }
            }
            if (gridPtr) {
                plotCanvasDock_->scene()->clearItems();
                auto heatmap = std::make_unique<plot::Heatmap>(
                    gridPtr,
                    plot::Colormap::builtin(plot::Colormap::Builtin::Viridis));
                heatmap->setName(label);
                plotCanvasDock_->scene()->addItem(std::move(heatmap));
                plotCanvasDock_->scene()->autoRange();
                plotCanvasDock_->canvas()->update();
                plotCanvasDock_->show();
            }
        }
    } else {
        // Hide plot dock for non-Grid2D datasets without visualization.
        plotCanvasDock_->hide();
    }

    currentDocPath_ = label;
    updateWindowTitle();
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

// ---- Sample Data Generators ----

void MainWindow::openSampleSine1D() {
    constexpr int kNumPoints = 628;
    constexpr double kStep = 2.0 * M_PI / kNumPoints;

    std::vector<double> xData;
    std::vector<double> yData;
    xData.reserve(kNumPoints);
    yData.reserve(kNumPoints);

    for (int i = 0; i < kNumPoints; ++i) {
        double x = i * kStep;
        xData.push_back(x);
        yData.push_back(std::sin(x));
    }

    auto xCol = std::make_shared<data::Rank1Dataset>(
        QStringLiteral("x"), data::Unit::dimensionless(), std::move(xData));
    auto yCol = std::make_shared<data::Rank1Dataset>(
        QStringLiteral("y"), data::Unit::dimensionless(), std::move(yData));

    auto bundle = std::make_shared<data::TabularBundle>();
    bundle->addColumn(std::move(xCol));
    bundle->addColumn(std::move(yCol));

    const QString key = QStringLiteral("sample://sine-1d");
    const auto* rawBundle = registry_->addDocument(key, std::move(bundle));
    showTabular(rawBundle, key);

    statusBar()->showMessage(tr("Sample: Sine 1D (%1 points)").arg(kNumPoints));
}

void MainWindow::openSampleGaussian2D() {
    constexpr std::size_t kSize = 256;
    constexpr double kSigma = 40.0;
    constexpr double kCenter = 128.0;

    std::vector<double> values(kSize * kSize);
    for (std::size_t y = 0; y < kSize; ++y) {
        for (std::size_t x = 0; x < kSize; ++x) {
            double dx = static_cast<double>(x) - kCenter;
            double dy = static_cast<double>(y) - kCenter;
            values[y * kSize + x] = std::exp(-(dx * dx + dy * dy) / (2.0 * kSigma * kSigma));
        }
    }

    data::Dimension dimX{QStringLiteral("x"), data::Unit::dimensionless(),
                         kSize, data::CoordinateArray(0.0, 1.0, kSize)};
    data::Dimension dimY{QStringLiteral("y"), data::Unit::dimensionless(),
                         kSize, data::CoordinateArray(0.0, 1.0, kSize)};

    auto grid = std::make_shared<data::Grid2D>(
        QStringLiteral("Gaussian 2D"), data::Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(values));

    sampleDatasets_.push_back(grid);
    showDataset(grid.get(), QStringLiteral("Gaussian 2D"));

    data::MemoryManager::instance().trackAllocation(grid->sizeBytes());
    statusBar()->showMessage(tr("Sample: Gaussian 2D (256x256)"));
    updateMemoryStatus();
}

void MainWindow::openSampleMandelbrot() {
    constexpr std::size_t kSize = 512;
    constexpr int kMaxIter = 256;
    constexpr double kXMin = -2.0;
    constexpr double kXMax = 1.0;
    constexpr double kYMin = -1.5;
    constexpr double kYMax = 1.5;

    std::vector<double> values(kSize * kSize);
    for (std::size_t py = 0; py < kSize; ++py) {
        for (std::size_t px = 0; px < kSize; ++px) {
            double cx = kXMin + (kXMax - kXMin) * static_cast<double>(px) / static_cast<double>(kSize - 1);
            double cy = kYMin + (kYMax - kYMin) * static_cast<double>(py) / static_cast<double>(kSize - 1);
            double zx = 0.0;
            double zy = 0.0;
            int iter = 0;
            while (zx * zx + zy * zy <= 4.0 && iter < kMaxIter) {
                double tmp = zx * zx - zy * zy + cx;
                zy = 2.0 * zx * zy + cy;
                zx = tmp;
                ++iter;
            }
            values[py * kSize + px] = static_cast<double>(iter);
        }
    }

    data::Dimension dimX{QStringLiteral("x"), data::Unit::dimensionless(),
                         kSize, data::CoordinateArray(kXMin, (kXMax - kXMin) / static_cast<double>(kSize - 1), kSize)};
    data::Dimension dimY{QStringLiteral("y"), data::Unit::dimensionless(),
                         kSize, data::CoordinateArray(kYMin, (kYMax - kYMin) / static_cast<double>(kSize - 1), kSize)};

    auto grid = std::make_shared<data::Grid2D>(
        QStringLiteral("Mandelbrot"), data::Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(values));

    sampleDatasets_.push_back(grid);
    showDataset(grid.get(), QStringLiteral("Mandelbrot"));

    data::MemoryManager::instance().trackAllocation(grid->sizeBytes());
    statusBar()->showMessage(tr("Sample: Mandelbrot (512x512)"));
    updateMemoryStatus();
}

void MainWindow::openSampleVolumeSphere() {
    constexpr std::size_t kSize = 64;
    constexpr double kRadius = 28.0;
    constexpr double kCenter = 32.0;

    std::vector<double> values(kSize * kSize * kSize);
    for (std::size_t z = 0; z < kSize; ++z) {
        for (std::size_t y = 0; y < kSize; ++y) {
            for (std::size_t x = 0; x < kSize; ++x) {
                double dx = static_cast<double>(x) - kCenter;
                double dy = static_cast<double>(y) - kCenter;
                double dz = static_cast<double>(z) - kCenter;
                double dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                values[(z * kSize + y) * kSize + x] = (dist <= kRadius) ? 1.0 : 0.0;
            }
        }
    }

    data::Dimension dimX{QStringLiteral("x"), data::Unit::dimensionless(),
                         kSize, data::CoordinateArray(0.0, 1.0, kSize)};
    data::Dimension dimY{QStringLiteral("y"), data::Unit::dimensionless(),
                         kSize, data::CoordinateArray(0.0, 1.0, kSize)};
    data::Dimension dimZ{QStringLiteral("z"), data::Unit::dimensionless(),
                         kSize, data::CoordinateArray(0.0, 1.0, kSize)};

    auto volume = std::make_shared<data::Volume3D>(
        QStringLiteral("Volume Sphere"), data::Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(dimZ), std::move(values));

    sampleDatasets_.push_back(volume);
    showDataset(volume.get(), QStringLiteral("Volume Sphere"));

    // Also show as VolumeItem in PlotCanvas3DDock.
    auto volItem = std::make_unique<plot3d::VolumeItem>(
        volume, QStringLiteral("Volume Sphere"));
    plotCanvas3D_->scene()->clearItems();
    plotCanvas3D_->addItem(std::move(volItem));
    plotCanvas3D_->setMinimumSize(200, 200);
    plotCanvas3D_->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    plotCanvas3D_->setVisible(true);
    {
        QList<int> sz = hSplitter_->sizes();
        int total = sz[0] + sz[1];
        sz[0] = total * 2 / 3;
        sz[1] = total / 3;
        hSplitter_->setSizes(sz);
    }

    data::MemoryManager::instance().trackAllocation(volume->sizeBytes());
    statusBar()->showMessage(tr("Sample: Volume Sphere (64x64x64)"));
    updateMemoryStatus();
}

// ---- Additional 2D samples ----

void MainWindow::openSampleScatter2D() {
    constexpr int kN = 200;
    std::vector<double> xData, yData;
    xData.reserve(kN); yData.reserve(kN);
    std::srand(42);
    for (int i = 0; i < kN; ++i) {
        double x = static_cast<double>(std::rand()) / RAND_MAX * 10.0;
        double y = 2.0 * x + 1.0 + (static_cast<double>(std::rand()) / RAND_MAX - 0.5) * 4.0;
        xData.push_back(x); yData.push_back(y);
    }
    auto xCol = std::make_shared<data::Rank1Dataset>("x", data::Unit::dimensionless(), std::move(xData));
    auto yCol = std::make_shared<data::Rank1Dataset>("y", data::Unit::dimensionless(), std::move(yData));
    auto bundle = std::make_shared<data::TabularBundle>();
    bundle->addColumn(std::move(xCol)); bundle->addColumn(std::move(yCol));
    const QString key = QStringLiteral("sample://scatter-2d");
    const auto* raw = registry_->addDocument(key, std::move(bundle));
    showTabular(raw, key);
    statusBar()->showMessage(tr("Sample: Scatter 2D (%1 points) — select Scatter type in Plot Type combo").arg(kN));
}

void MainWindow::openSampleBarChart() {
    std::vector<double> categories = {1, 2, 3, 4, 5, 6};
    std::vector<double> values = {23, 45, 12, 67, 34, 56};
    auto xCol = std::make_shared<data::Rank1Dataset>("category", data::Unit::dimensionless(), std::move(categories));
    auto yCol = std::make_shared<data::Rank1Dataset>("value", data::Unit::dimensionless(), std::move(values));
    auto bundle = std::make_shared<data::TabularBundle>();
    bundle->addColumn(std::move(xCol)); bundle->addColumn(std::move(yCol));
    const QString key = QStringLiteral("sample://bar-chart");
    const auto* raw = registry_->addDocument(key, std::move(bundle));
    showTabular(raw, key);
    statusBar()->showMessage(tr("Sample: Bar Chart — select Bar type in Plot Type combo"));
}

void MainWindow::openSampleHistogram() {
    constexpr int kN = 1000;
    std::vector<double> values;
    values.reserve(kN);
    std::srand(123);
    for (int i = 0; i < kN; ++i) {
        // Box-Muller for normal distribution
        double u1 = (static_cast<double>(std::rand()) + 1.0) / (RAND_MAX + 1.0);
        double u2 = static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
        double z = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
        values.push_back(z * 15.0 + 50.0); // mean=50, std=15
    }
    auto col = std::make_shared<data::Rank1Dataset>("measurement", data::Unit::dimensionless(), std::move(values));
    auto bundle = std::make_shared<data::TabularBundle>();
    bundle->addColumn(std::move(col));
    const QString key = QStringLiteral("sample://histogram");
    const auto* raw = registry_->addDocument(key, std::move(bundle));
    showTabular(raw, key);
    statusBar()->showMessage(tr("Sample: Histogram (normal dist, N=1000) — select Histogram type"));
}

void MainWindow::openSampleBoxPlot() {
    std::vector<double> groupA = {2, 3, 5, 7, 8, 9, 10, 11, 12, 15, 20};
    std::vector<double> groupB = {5, 8, 10, 12, 13, 14, 15, 16, 18, 22, 25};
    auto colA = std::make_shared<data::Rank1Dataset>("Group A", data::Unit::dimensionless(), std::move(groupA));
    auto colB = std::make_shared<data::Rank1Dataset>("Group B", data::Unit::dimensionless(), std::move(groupB));
    auto bundle = std::make_shared<data::TabularBundle>();
    bundle->addColumn(std::move(colA)); bundle->addColumn(std::move(colB));
    const QString key = QStringLiteral("sample://boxplot");
    const auto* raw = registry_->addDocument(key, std::move(bundle));
    showTabular(raw, key);
    statusBar()->showMessage(tr("Sample: Box Plot — select BoxPlot type"));
}

void MainWindow::openSampleViolin() {
    constexpr int kN = 500;
    std::vector<double> bimodal;
    bimodal.reserve(kN);
    std::srand(456);
    for (int i = 0; i < kN; ++i) {
        double u1 = (static_cast<double>(std::rand()) + 1.0) / (RAND_MAX + 1.0);
        double u2 = static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
        double z = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
        bimodal.push_back(i < kN / 2 ? z * 5.0 + 30.0 : z * 8.0 + 60.0);
    }
    auto col = std::make_shared<data::Rank1Dataset>("bimodal", data::Unit::dimensionless(), std::move(bimodal));
    auto bundle = std::make_shared<data::TabularBundle>();
    bundle->addColumn(std::move(col));
    const QString key = QStringLiteral("sample://violin");
    const auto* raw = registry_->addDocument(key, std::move(bundle));
    showTabular(raw, key);
    statusBar()->showMessage(tr("Sample: Violin Plot (bimodal dist) — select Violin type"));
}

// ---- 3D samples ----

void MainWindow::openSampleScatter3D() {
    constexpr int kN = 500;
    std::vector<double> xd, yd, zd;
    xd.reserve(kN); yd.reserve(kN); zd.reserve(kN);
    std::srand(789);
    for (int i = 0; i < kN; ++i) {
        double theta = static_cast<double>(std::rand()) / RAND_MAX * 2.0 * M_PI;
        double phi = std::acos(2.0 * static_cast<double>(std::rand()) / RAND_MAX - 1.0);
        double r = 1.0 + 0.3 * static_cast<double>(std::rand()) / RAND_MAX;
        xd.push_back(r * std::sin(phi) * std::cos(theta));
        yd.push_back(r * std::sin(phi) * std::sin(theta));
        zd.push_back(r * std::cos(phi));
    }
    auto xCol = std::make_shared<data::Rank1Dataset>("x", data::Unit::dimensionless(), std::move(xd));
    auto yCol = std::make_shared<data::Rank1Dataset>("y", data::Unit::dimensionless(), std::move(yd));
    auto zCol = std::make_shared<data::Rank1Dataset>("z", data::Unit::dimensionless(), std::move(zd));

    // Show in 3D dock.
    auto scatter = std::make_unique<plot3d::Scatter3D>(
        xCol, yCol, zCol, Qt::blue, QStringLiteral("Scatter3D"));
    plotCanvas3D_->scene()->clearItems();
    plotCanvas3D_->addItem(std::move(scatter));
    plotCanvas3D_->setMinimumSize(200, 200);
    plotCanvas3D_->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    plotCanvas3D_->setVisible(true);
    {
        QList<int> sz = hSplitter_->sizes();
        int total = sz[0] + sz[1];
        sz[0] = total * 2 / 3;
        sz[1] = total / 3;
        hSplitter_->setSizes(sz);
    }

    // Show x,y,z in data table (A6 fix: was missing TabularBundle).
    auto bundle = std::make_shared<data::TabularBundle>();
    bundle->addColumn(xCol);
    bundle->addColumn(yCol);
    bundle->addColumn(zCol);
    const QString key = QStringLiteral("sample://scatter-3d");
    const auto* rawBundle = registry_->addDocument(key, std::move(bundle));
    showTabular(rawBundle, key);

    statusBar()->showMessage(tr("Sample: Scatter3D (%1 points on sphere shell)").arg(kN));
}

void MainWindow::openSampleSurface3D() {
    // Generate Gaussian 2D grid for the surface.
    constexpr std::size_t kSize = 64;
    constexpr double kSigma = 20.0;
    constexpr double kCenter = 32.0;

    std::vector<double> values(kSize * kSize);
    for (std::size_t y = 0; y < kSize; ++y) {
        for (std::size_t x = 0; x < kSize; ++x) {
            double dx = static_cast<double>(x) - kCenter;
            double dy = static_cast<double>(y) - kCenter;
            values[y * kSize + x] = std::exp(-(dx * dx + dy * dy) / (2.0 * kSigma * kSigma));
        }
    }

    data::Dimension dimX{QStringLiteral("x"), data::Unit::dimensionless(),
                         kSize, data::CoordinateArray(0.0, 1.0, kSize)};
    data::Dimension dimY{QStringLiteral("y"), data::Unit::dimensionless(),
                         kSize, data::CoordinateArray(0.0, 1.0, kSize)};

    auto grid = std::make_shared<data::Grid2D>(
        QStringLiteral("Surface Gaussian"), data::Unit::dimensionless(),
        std::move(dimX), std::move(dimY), std::move(values));

    sampleDatasets_.push_back(grid);

    auto surface = std::make_unique<plot3d::Surface3D>(
        grid, QColor(100, 149, 237), QStringLiteral("Surface3D"));
    surface->setColormap(std::make_shared<plot::Colormap>(
        plot::Colormap::builtin(plot::Colormap::Builtin::Viridis)));
    plotCanvas3D_->scene()->clearItems();
    plotCanvas3D_->addItem(std::move(surface));
    plotCanvas3D_->setMinimumSize(200, 200);
    plotCanvas3D_->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    plotCanvas3D_->setVisible(true);
    {
        QList<int> sz = hSplitter_->sizes();
        int total = sz[0] + sz[1];
        sz[0] = total * 2 / 3;
        sz[1] = total / 3;
        hSplitter_->setSizes(sz);
    }

    showDataset(grid.get(), QStringLiteral("sample://surface-3d"));
    statusBar()->showMessage(tr("Sample: Surface3D (Gaussian 64x64)"));
}

void MainWindow::openSampleStreamlines() {
    // Streamlines require a vector field; show placeholder message for now.
    plotCanvas3D_->setMinimumSize(200, 200);
    plotCanvas3D_->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    plotCanvas3D_->setVisible(true);
    {
        QList<int> sz = hSplitter_->sizes();
        int total = sz[0] + sz[1];
        sz[0] = total * 2 / 3;
        sz[1] = total / 3;
        hSplitter_->setSizes(sz);
    }
    statusBar()->showMessage(tr("Sample: Streamlines — requires 3D vector field data (not yet generated)"));
}

void MainWindow::openSampleIsosurface() {
    // Load Volume Sphere and show it as a VolumeItem in the 3D dock.
    openSampleVolumeSphere();
    statusBar()->showMessage(tr("Sample: Isosurface — Volume Sphere loaded in 3D View"));
}

// ---- Memory Budget Dialog (T24) ----

void MainWindow::showMemoryBudgetDialog() {
    auto* dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Preferences"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    auto* layout = new QFormLayout(dialog);

    auto* spinBox = new QDoubleSpinBox(dialog);
    spinBox->setRange(0.25, 64.0);
    spinBox->setSingleStep(0.25);
    spinBox->setDecimals(2);
    spinBox->setSuffix(QStringLiteral(" GB"));

    // Current budget in GB
    double currentGB = static_cast<double>(data::MemoryManager::instance().memoryBudgetBytes()) / kOneGB;
    spinBox->setValue(currentGB);

    layout->addRow(tr("Memory Budget:"), spinBox);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    layout->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, dialog, [this, spinBox, dialog]() {
        double gb = spinBox->value();
        auto bytes = static_cast<std::size_t>(gb * kOneGB);
        data::MemoryManager::instance().setBudget(bytes);
        updateMemoryStatus();
        dialog->accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    dialog->exec();
}

// ---- Geometry / Close / About / Save ----

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
           "<p>Phase 6 — I/O Loaders, Memory Management, and Dataset Types.</p>")
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
        QString name = fi.fileName();
        if (currentDocPath_.startsWith(QStringLiteral("sample://")))
            name = currentDocPath_.mid(9);
        title = QStringLiteral("%1 — Lumen").arg(name);
        if (workspaceManager_ != nullptr &&
            workspaceManager_->isModified(currentDocPath_)) {
            title += QStringLiteral(" \u25CF");
        }
    }
    setWindowTitle(title);
}

}  // namespace lumen
