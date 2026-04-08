#pragma once

#include <QMainWindow>

namespace lumen {

/// Main application window.
///
/// Phase 0: empty window with a menu bar and an About dialog.
/// Future phases add docks, plot canvas, file explorer, etc.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildMenus();
    void restoreGeometry();
    void saveGeometry() const;
    void showAbout();
};

}  // namespace lumen
