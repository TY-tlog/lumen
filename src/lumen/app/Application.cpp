#include "Application.h"

#include "../core/CommandBus.h"
#include "../core/DocumentRegistry.h"
#include "../core/EventBus.h"
#include "../core/PlotRegistry.h"
#include "../core/io/WorkspaceManager.h"
#include "../style/StyleManager.h"
#include "../ui/MainWindow.h"

#include <QFile>
#include <QFontDatabase>
#include <QString>
#include <QStyleHints>

namespace lumen {

namespace {
constexpr auto kOrganizationName = "Lumen";
constexpr auto kApplicationName = "Lumen";
constexpr auto kApplicationVersion = "0.0.1";
}  // namespace

Application::Application(int argc, char* argv[])
    : qapp_(argc, argv) {
    configureApplication();
    loadStyleSheet();

    eventBus_ = std::make_unique<core::EventBus>();
    commandBus_ = std::make_unique<core::CommandBus>();
    documentRegistry_ =
        std::make_unique<core::DocumentRegistry>(eventBus_.get());
    plotRegistry_ =
        std::make_unique<core::PlotRegistry>(eventBus_.get());
    workspaceManager_ = std::make_unique<core::io::WorkspaceManager>(
        documentRegistry_.get(), plotRegistry_.get(), commandBus_.get());
    mainWindow_ = std::make_unique<MainWindow>(
        documentRegistry_.get(), plotRegistry_.get(), commandBus_.get(),
        workspaceManager_.get());
}

Application::~Application() = default;

void Application::configureApplication() {
    QApplication::setOrganizationName(kOrganizationName);
    QApplication::setApplicationName(kApplicationName);
    QApplication::setApplicationVersion(kApplicationVersion);

    // High-DPI is automatic in Qt 6; no setAttribute needed.
}

void Application::loadStyleSheet() {
    StyleManager styleManager;
    styleManager.apply();
}

int Application::run() {
    mainWindow_->show();
    return QApplication::exec();
}

}  // namespace lumen
