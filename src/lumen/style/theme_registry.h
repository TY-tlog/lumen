#pragma once

#include "json_io.h"
#include "types.h"

#include <QHash>
#include <QString>
#include <QStringList>

namespace lumen::style {

/// Registry of built-in and user themes.
///
/// Built-in themes are loaded at construction from resources/themes/.
/// They are immutable — modification attempts return false.
/// User themes can be added/removed at runtime.
class ThemeRegistry {
public:
    ThemeRegistry();

    /// Load built-in themes from resources/themes/*.lumen-style.json.
    void loadBuiltinThemes();

    /// Get a theme by name. Returns empty Style if not found.
    [[nodiscard]] Style theme(const QString& name) const;

    /// Check if a theme exists.
    [[nodiscard]] bool hasTheme(const QString& name) const;

    /// Check if a theme is built-in (immutable).
    [[nodiscard]] bool isBuiltin(const QString& name) const;

    /// List all theme names (built-in first, then user).
    [[nodiscard]] QStringList themeNames() const;

    /// List built-in theme names only.
    [[nodiscard]] QStringList builtinNames() const;

    /// Register a user theme. Returns false if name conflicts with builtin.
    bool registerUserTheme(const QString& name, const Style& style);

    /// Remove a user theme. Returns false if builtin or not found.
    bool removeUserTheme(const QString& name);

    /// Set the active theme name.
    void setActiveTheme(const QString& name);
    [[nodiscard]] QString activeTheme() const;

    /// Get the active theme's Style.
    [[nodiscard]] Style activeStyle() const;

private:
    QHash<QString, Style> builtins_;
    QHash<QString, Style> userThemes_;
    QStringList builtinOrder_;
    QString activeTheme_ = QStringLiteral("lumen-light");
};

}  // namespace lumen::style
