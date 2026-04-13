#include "ColorPipeline.h"

#include <QColorSpace>

namespace lumen::exp {

void ColorPipeline::applyProfileToImage(QImage& image, const ColorProfile& profile)
{
    if (!profile.isValid())
        return;

    QByteArray iccData = profile.iccBytes();
    if (iccData.isEmpty())
        return;

    QColorSpace cs = QColorSpace::fromIccProfile(iccData);
    if (cs.isValid()) {
        image.setColorSpace(cs);
    }
}

QByteArray ColorPipeline::profileBytesForPdf(const ColorProfile& profile)
{
    if (!profile.isValid())
        return {};
    return profile.iccBytes();
}

bool ColorPipeline::needsConversion(const ColorProfile& profile)
{
    if (!profile.isValid())
        return false;
    // sRGB is the default working space — no conversion needed.
    return profile.name() != QStringLiteral("sRGB IEC61966-2.1");
}

}  // namespace lumen::exp
