#include "Application.h"

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
    mainWindow_ = std::make_unique<MainWindow>();
}

Application::~Application() = default;

void Application::configureApplication() {
    QApplication::setOrganizationName(kOrganizationName);
    QApplication::setApplicationName(kApplicationName);
    QApplication::setApplicationVersion(kApplicationVersion);

    // High-DPI is automatic in Qt 6; no setAttribute needed.
}

void Application::loadStyleSheet() {
    // Phase 0 placeholder: no stylesheet yet. Phase 1 introduces the
    // Apple-mood design system.
}

int Application::run() {
    mainWindow_->show();
    return QApplication::exec();
}

}  // namespace lumen
