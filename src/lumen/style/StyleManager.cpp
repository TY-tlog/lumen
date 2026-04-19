#include "StyleManager.h"

#include "DesignTokens.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QStringList>
#include <QTextStream>

namespace lumen {

namespace {

constexpr auto kFontFamily = "Inter";
constexpr int kDefaultFontSize = tokens::typography::body.sizePx;

// Font resource paths (embedded via qt_add_resources)
const QStringList kFontResources{
    QStringLiteral(":/fonts/Inter-Regular.ttf"),
    QStringLiteral(":/fonts/Inter-Medium.ttf"),
    QStringLiteral(":/fonts/Inter-SemiBold.ttf"),
    QStringLiteral(":/fonts/Inter-Bold.ttf"),
};

// Fallback font stack matching design-system.md
constexpr auto kFallbackFamily = "Helvetica Neue";

}  // namespace

StyleManager::StyleManager()
    : fontFamily_(kFallbackFamily) {
}

void StyleManager::apply() {
    loadFonts();
    applyStyleSheet();
}

QFont StyleManager::defaultFont() const {
    QFont font(fontFamily_);
    font.setPixelSize(kDefaultFontSize);
    font.setWeight(tokens::typography::body.weight);
    return font;
}

void StyleManager::loadFonts() {
    bool anyLoaded = false;

    for (const auto& path : kFontResources) {
        int id = QFontDatabase::addApplicationFont(path);
        if (id < 0) {
            qWarning() << "StyleManager: failed to load font:" << path;
        } else {
            anyLoaded = true;
            const auto families = QFontDatabase::applicationFontFamilies(id);
            if (!families.isEmpty()) {
                qInfo() << "StyleManager: loaded font family:"
                        << families.first() << "from" << path;
            }
        }
    }

    if (anyLoaded) {
        fontFamily_ = kFontFamily;
    } else {
        qWarning() << "StyleManager: no Inter fonts loaded, falling back to"
                    << fontFamily_;
    }

    // Set application-wide default font
    QFont appFont(fontFamily_);
    appFont.setPixelSize(kDefaultFontSize);
    appFont.setWeight(tokens::typography::body.weight);
    QApplication::setFont(appFont);
}

void StyleManager::applyStyleSheet() {
    setDarkMode(darkMode_);
}

void StyleManager::setDarkMode(bool dark) {
    darkMode_ = dark;
    QString path = dark ? QStringLiteral(":/styles/dark.qss")
                        : QStringLiteral(":/styles/light.qss");
    QFile qssFile(path);
    if (!qssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "StyleManager: could not load" << path;
        return;
    }

    QTextStream stream(&qssFile);
    QString styleSheet = stream.readAll();
    qssFile.close();

    qApp->setStyleSheet(styleSheet);
    qInfo() << "StyleManager: applied" << (dark ? "dark" : "light")
            << "stylesheet (" << styleSheet.size() << "chars)";
}

}  // namespace lumen
