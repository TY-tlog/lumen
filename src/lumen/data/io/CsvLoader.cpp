#include "CsvLoader.h"

#include <data/CsvReader.h>

namespace lumen::data::io {

QStringList CsvLoader::supportedExtensions() const
{
    return {QStringLiteral("csv"), QStringLiteral("tsv")};
}

std::shared_ptr<TabularBundle> CsvLoader::loadTabular(const QString& path)
{
    CsvReaderOptions opts;
    if (path.endsWith(QStringLiteral(".tsv"), Qt::CaseInsensitive)) {
        opts.delimiter = '\t';
    }
    CsvReader reader(opts);
    auto bundle = std::make_shared<TabularBundle>(reader.readFile(path));
    return bundle;
}

} // namespace lumen::data::io
