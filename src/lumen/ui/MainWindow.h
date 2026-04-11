#pragma once

#include <QMainWindow>
#include <QStringList>

class QLabel;
class QMenu;
class QTimer;

namespace lumen::core {
class CommandBus;
class DocumentRegistry;
class PlotRegistry;
}  // namespace lumen::core

namespace lumen::core::io {
class WorkspaceManager;
}  // namespace lumen::core::io

namespace lumen::data {
class Dataset;
class TabularBundle;
}  // namespace lumen::data

namespace lumen::ui {
class DataTableDock;
class PlotCanvasDock;
}  // namespace lumen::ui

namespace lumen {

/// Main application window.
///
/// Phase 0: empty window with a menu bar and an About dialog.
/// Phase 1: adds DataTableDock for CSV data display, file-open flow,
///          recent files submenu, and status bar feedback.
/// Phase 6.3: universal File Open, sample menu, memory status bar.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /// Construct the main window.
    /// @p registry must outlive the window; owned by Application.
    explicit MainWindow(core::DocumentRegistry* registry,
                        core::PlotRegistry* plotRegistry = nullptr,
                        core::CommandBus* commandBus = nullptr,
                        core::io::WorkspaceManager* workspaceManager = nullptr,
                        QWidget* parent = nullptr);
    ~MainWindow() override;

    /// Access the data table dock (for integration with file loading).
    [[nodiscard]] ui::DataTableDock* dataTableDock() const { return dataTableDock_; }
    [[nodiscard]] ui::PlotCanvasDock* plotCanvasDock() const { return plotCanvasDock_; }

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onSaveWorkspace();
    void onSaveWorkspaceAs();
    void onRevertToSaved();
    void onExportFigure();
    void updateMemoryStatus();

private:
    void buildMenus();
    void buildSampleMenu(QMenu* fileMenu);
    void setupMemoryStatusBar();
    void restoreGeometry();
    void saveGeometry() const;
    void showAbout();
    void updateWindowTitle();

    /// Open a file via QFileDialog (universal — all registered formats).
    void openFile();

    /// Load the given file path (shared by open and recent files).
    void loadFile(const QString& filePath);

    /// Show a TabularBundle in the data table dock and auto-plot.
    void showTabular(const data::TabularBundle* bundle, const QString& path);

    /// Show a Dataset (Grid2D/Volume3D) as a placeholder in the data table dock.
    void showDataset(const data::Dataset* ds, const QString& label);

    /// Add a path to the recent-files list and refresh the submenu.
    void addRecentFile(const QString& filePath);

    /// Rebuild the recent-files submenu from QSettings.
    void updateRecentFilesMenu();

    // Sample data generators
    void openSampleSine1D();
    void openSampleGaussian2D();
    void openSampleMandelbrot();
    void openSampleVolumeSphere();

    /// Show memory budget settings dialog.
    void showMemoryBudgetDialog();

    core::DocumentRegistry* registry_ = nullptr;
    core::CommandBus* commandBus_ = nullptr;
    core::io::WorkspaceManager* workspaceManager_ = nullptr;
    ui::DataTableDock* dataTableDock_ = nullptr;
    ui::PlotCanvasDock* plotCanvasDock_ = nullptr;
    QMenu* recentFilesMenu_ = nullptr;
    QString currentDocPath_;
    QLabel* memoryLabel_ = nullptr;
    QTimer* memoryTimer_ = nullptr;

    static constexpr int kMaxRecentFiles = 10;
};

}  // namespace lumen
