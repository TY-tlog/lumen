#pragma once

#include <QApplication>

#include <memory>

namespace lumen {

class MainWindow;

/// Top-level application object.
///
/// Owns the QApplication and the main window. Responsible for global
/// setup (style sheet, fonts, settings) and the event-loop entry point.
class Application {
public:
    Application(int argc, char* argv[]);
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;
    ~Application();

    /// Show the main window and run the Qt event loop.
    /// Returns the application exit code.
    int run();

private:
    void configureApplication();
    void loadStyleSheet();

    QApplication qapp_;
    std::unique_ptr<MainWindow> mainWindow_;
};

}  // namespace lumen
