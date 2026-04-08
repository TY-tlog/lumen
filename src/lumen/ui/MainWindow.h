#pragma once

#include <QMainWindow>

namespace lumen::ui {
class DataTableDock;
}  // namespace lumen::ui

namespace lumen {

/// Main application window.
///
/// Phase 0: empty window with a menu bar and an About dialog.
/// Phase 1: adds DataTableDock for CSV data display.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    /// Access the data table dock (for integration with file loading).
    [[nodiscard]] ui::DataTableDock* dataTableDock() const { return dataTableDock_; }

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildMenus();
    void restoreGeometry();
    void saveGeometry() const;
    void showAbout();

    ui::DataTableDock* dataTableDock_ = nullptr;
};

}  // namespace lumen
