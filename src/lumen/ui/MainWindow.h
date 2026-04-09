#pragma once

#include <QMainWindow>
#include <QStringList>

class QMenu;

namespace lumen::core {
class CommandBus;
class DocumentRegistry;
class PlotRegistry;
}  // namespace lumen::core

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
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /// Construct the main window.
    /// @p registry must outlive the window; owned by Application.
    explicit MainWindow(core::DocumentRegistry* registry,
                        core::PlotRegistry* plotRegistry = nullptr,
                        core::CommandBus* commandBus = nullptr,
                        QWidget* parent = nullptr);
    ~MainWindow() override;

    /// Access the data table dock (for integration with file loading).
    [[nodiscard]] ui::DataTableDock* dataTableDock() const { return dataTableDock_; }
    [[nodiscard]] ui::PlotCanvasDock* plotCanvasDock() const { return plotCanvasDock_; }

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildMenus();
    void restoreGeometry();
    void saveGeometry() const;
    void showAbout();

    /// Open a CSV file via QFileDialog.
    void openCsvFile();

    /// Load the given CSV file path (shared by open and recent files).
    void loadFile(const QString& filePath);

    /// Add a path to the recent-files list and refresh the submenu.
    void addRecentFile(const QString& filePath);

    /// Rebuild the recent-files submenu from QSettings.
    void updateRecentFilesMenu();

    core::DocumentRegistry* registry_ = nullptr;
    core::CommandBus* commandBus_ = nullptr;
    ui::DataTableDock* dataTableDock_ = nullptr;
    ui::PlotCanvasDock* plotCanvasDock_ = nullptr;
    QMenu* recentFilesMenu_ = nullptr;

    static constexpr int kMaxRecentFiles = 10;
};

}  // namespace lumen
