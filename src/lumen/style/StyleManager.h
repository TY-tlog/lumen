#pragma once

#include <QFont>
#include <QString>

class QApplication;

namespace lumen {

/// Manages the application's visual style: fonts, QSS stylesheet, palette.
///
/// Created once by Application during startup. Not a true singleton—
/// Application owns its lifetime. Loads Inter font files from Qt
/// resources and applies the light-theme QSS stylesheet.
class StyleManager {
public:
    StyleManager();

    StyleManager(const StyleManager&) = delete;
    StyleManager& operator=(const StyleManager&) = delete;
    StyleManager(StyleManager&&) = delete;
    StyleManager& operator=(StyleManager&&) = delete;

    ~StyleManager() = default;

    /// Load fonts, generate stylesheet, and apply it to qApp.
    void apply();

    /// Switch to dark or light QSS at runtime.
    void setDarkMode(bool dark);
    [[nodiscard]] bool isDarkMode() const { return darkMode_; }

    /// Returns the application default font (Inter or fallback).
    [[nodiscard]] QFont defaultFont() const;

private:
    void loadFonts();
    void applyStyleSheet();

    QString fontFamily_;
    bool darkMode_ = false;
};

}  // namespace lumen
