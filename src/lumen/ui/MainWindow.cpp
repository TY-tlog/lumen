#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QCloseEvent>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <Qt>

namespace lumen {

namespace {
constexpr int kDefaultWidth = 1400;
constexpr int kDefaultHeight = 900;
constexpr auto kGeometryKey = "MainWindow/geometry";
constexpr auto kStateKey = "MainWindow/state";
}  // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("Lumen");
    resize(kDefaultWidth, kDefaultHeight);

    auto* placeholder = new QLabel(
        "Lumen — Phase 0\n\nNo features yet. See docs/specs/phase-0-spec.md.",
        this);
    placeholder->setAlignment(Qt::AlignCenter);
    setCentralWidget(placeholder);

    buildMenus();
    restoreGeometry();
}

MainWindow::~MainWindow() = default;

void MainWindow::buildMenus() {
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    auto* quitAction = fileMenu->addAction(tr("&Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    Q_UNUSED(viewMenu);

    auto* helpMenu = menuBar()->addMenu(tr("&Help"));
    auto* aboutAction = helpMenu->addAction(tr("&About Lumen"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
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
           "<p>Phase 0 — Foundation.</p>")
            .arg(QApplication::applicationVersion()));
}

}  // namespace lumen
