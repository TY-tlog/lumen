#include "LoaderRegistry.h"

#include <QFileInfo>

namespace lumen::data::io {

LoaderRegistry& LoaderRegistry::instance()
{
    static LoaderRegistry registry;
    return registry;
}

void LoaderRegistry::registerLoader(std::unique_ptr<DatasetLoader> loader)
{
    if (loader) {
        loaders_.push_back(std::move(loader));
    }
}

DatasetLoader* LoaderRegistry::loaderForPath(const QString& path) const
{
    QFileInfo info(path);
    QString ext = info.suffix().toLower();

    for (const auto& loader : loaders_) {
        if (loader->supportedExtensions().contains(ext)) {
            return loader.get();
        }
    }
    return nullptr;
}

QStringList LoaderRegistry::allSupportedExtensions() const
{
    QStringList all;
    for (const auto& loader : loaders_) {
        all.append(loader->supportedExtensions());
    }
    all.removeDuplicates();
    return all;
}

QString LoaderRegistry::fileFilter() const
{
    QStringList filters;
    for (const auto& loader : loaders_) {
        QStringList exts = loader->supportedExtensions();
        if (exts.isEmpty()) {
            continue;
        }
        // Build label from first extension, uppercased
        QString label = exts.first().toUpper();
        QStringList patterns;
        for (const QString& ext : exts) {
            patterns.append(QStringLiteral("*.") + ext);
        }
        filters.append(label + QStringLiteral(" (") + patterns.join(QStringLiteral(" "))
                        + QStringLiteral(")"));
    }

    // Add an "All Supported" entry at the front
    if (!filters.isEmpty()) {
        QStringList allPatterns;
        for (const auto& loader : loaders_) {
            for (const QString& ext : loader->supportedExtensions()) {
                allPatterns.append(QStringLiteral("*.") + ext);
            }
        }
        allPatterns.removeDuplicates();
        filters.prepend(QStringLiteral("All Supported (") + allPatterns.join(QStringLiteral(" "))
                         + QStringLiteral(")"));
    }

    return filters.join(QStringLiteral(";;"));
}

int LoaderRegistry::loaderCount() const
{
    return static_cast<int>(loaders_.size());
}

void LoaderRegistry::clear()
{
    loaders_.clear();
}

} // namespace lumen::data::io
