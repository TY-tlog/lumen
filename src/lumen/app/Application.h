#pragma once

#include <QApplication>

#include <memory>

namespace lumen::core {
class CommandBus;
class EventBus;
class DocumentRegistry;
class PlotRegistry;
}  // namespace lumen::core

namespace lumen::core::io {
class WorkspaceManager;
}  // namespace lumen::core::io

namespace lumen {

class MainWindow;

/// Top-level application object.
///
/// Owns the QApplication, the main window, and global core services
/// (EventBus, DocumentRegistry). Responsible for global setup (style
/// sheet, fonts, settings) and the event-loop entry point.
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
    std::unique_ptr<core::EventBus> eventBus_;
    std::unique_ptr<core::CommandBus> commandBus_;
    std::unique_ptr<core::DocumentRegistry> documentRegistry_;
    std::unique_ptr<core::PlotRegistry> plotRegistry_;
    std::unique_ptr<core::io::WorkspaceManager> workspaceManager_;
    std::unique_ptr<MainWindow> mainWindow_;
};

}  // namespace lumen
