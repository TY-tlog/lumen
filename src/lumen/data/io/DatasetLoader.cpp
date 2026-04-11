#include "DatasetLoader.h"

#include <QFileInfo>

namespace lumen::data::io {

std::shared_ptr<Dataset> DatasetLoader::loadDataset(const QString& /*path*/)
{
    return nullptr;
}

bool DatasetLoader::canLoad(const QString& path) const
{
    QFileInfo info(path);
    QString ext = info.suffix().toLower();
    return supportedExtensions().contains(ext);
}

} // namespace lumen::data::io
