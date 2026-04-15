#include "theme_registry.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace lumen::style {

ThemeRegistry::ThemeRegistry()
{
    loadBuiltinThemes();
}

void ThemeRegistry::loadBuiltinThemes()
{
    // Load from resources/themes/ directory.
    static const QStringList kThemeFiles = {
        QStringLiteral("lumen-light"),
        QStringLiteral("lumen-dark"),
        QStringLiteral("publication"),
        QStringLiteral("colorblind-safe"),
        QStringLiteral("presentation"),
        QStringLiteral("print-bw"),
    };

    // Try multiple paths (installed vs development).
    QStringList searchPaths = {
        QStringLiteral(":/themes"),
        QStringLiteral("resources/themes"),
        QStringLiteral("../resources/themes"),
    };

    for (const auto& name : kThemeFiles) {
        bool loaded = false;
        for (const auto& dir : searchPaths) {
            QString path = QStringLiteral("%1/%2.lumen-style.json").arg(dir, name);
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly))
                continue;

            QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
            if (doc.isNull())
                continue;

            QString err = validateStyleJson(doc.object());
            if (!err.isEmpty()) {
                qWarning() << "Theme" << name << "validation failed:" << err;
                continue;
            }

            builtins_[name] = loadStyleFromJson(doc.object());
            builtinOrder_.append(name);
            loaded = true;
            break;
        }

        if (!loaded) {
            // Register empty placeholder so the name is known.
            builtins_[name] = Style{};
            builtinOrder_.append(name);
        }
    }
}

Style ThemeRegistry::theme(const QString& name) const
{
    auto it = builtins_.find(name);
    if (it != builtins_.end())
        return it.value();
    auto uit = userThemes_.find(name);
    if (uit != userThemes_.end())
        return uit.value();
    return {};
}

bool ThemeRegistry::hasTheme(const QString& name) const
{
    return builtins_.contains(name) || userThemes_.contains(name);
}

bool ThemeRegistry::isBuiltin(const QString& name) const
{
    return builtins_.contains(name);
}

QStringList ThemeRegistry::themeNames() const
{
    QStringList result = builtinOrder_;
    for (auto it = userThemes_.begin(); it != userThemes_.end(); ++it)
        result.append(it.key());
    return result;
}

QStringList ThemeRegistry::builtinNames() const
{
    return builtinOrder_;
}

bool ThemeRegistry::registerUserTheme(const QString& name, const Style& style)
{
    if (builtins_.contains(name))
        return false;
    userThemes_[name] = style;
    return true;
}

bool ThemeRegistry::removeUserTheme(const QString& name)
{
    if (builtins_.contains(name))
        return false;
    return userThemes_.remove(name) > 0;
}

void ThemeRegistry::setActiveTheme(const QString& name)
{
    activeTheme_ = name;
}

QString ThemeRegistry::activeTheme() const
{
    return activeTheme_;
}

Style ThemeRegistry::activeStyle() const
{
    return theme(activeTheme_);
}

}  // namespace lumen::style
