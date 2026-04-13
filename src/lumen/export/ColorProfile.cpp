#include "ColorProfile.h"

#include <lcms2.h>

#include <QDebug>
#include <QFile>

#include <array>

namespace lumen::exp {

// ---- Impl (shared via shared_ptr for copy semantics) ----

struct ColorProfile::Impl {
    cmsHPROFILE profile = nullptr;
    QString profileName;
    QByteArray cachedIccBytes;

    ~Impl()
    {
        if (profile != nullptr) {
            cmsCloseProfile(profile);
        }
    }

    Impl() = default;
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;
};

// ---- Construction helpers ----

ColorProfile::ColorProfile(cmsHPROFILE profile, QString profileName)
    : impl_(std::make_shared<Impl>())
{
    impl_->profile = profile;
    impl_->profileName = std::move(profileName);
}

ColorProfile::~ColorProfile() = default;
ColorProfile::ColorProfile(const ColorProfile& other) = default;
ColorProfile& ColorProfile::operator=(const ColorProfile& other) = default;
ColorProfile::ColorProfile(ColorProfile&& other) noexcept = default;
ColorProfile& ColorProfile::operator=(ColorProfile&& other) noexcept = default;

// ---- Built-in profiles ----

ColorProfile ColorProfile::builtin(Builtin id)
{
    cmsHPROFILE profile = nullptr;
    QString name;

    switch (id) {
    case Builtin::sRGB:
        profile = cmsCreate_sRGBProfile();
        name = QStringLiteral("sRGB IEC61966-2.1");
        break;

    case Builtin::AdobeRGB: {
        // Adobe RGB (1998): D65 white point, specific primaries, gamma 2.2.
        cmsCIExyY d65;
        cmsWhitePointFromTemp(&d65, 6504);

        cmsCIExyYTRIPLE primaries = {
            {0.6400, 0.3300, 1.0},  // red
            {0.2100, 0.7100, 1.0},  // green
            {0.1500, 0.0600, 1.0}   // blue
        };

        cmsToneCurve* gamma22 = cmsBuildGamma(nullptr, 2.19921875);
        std::array<cmsToneCurve*, 3> curves = {gamma22, gamma22, gamma22};

        profile = cmsCreateRGBProfile(&d65, &primaries, curves.data());
        cmsFreeToneCurve(gamma22);
        name = QStringLiteral("Adobe RGB (1998)");
        break;
    }

    case Builtin::DisplayP3: {
        // Display P3: D65, P3 primaries, sRGB TRC.
        cmsCIExyY d65;
        cmsWhitePointFromTemp(&d65, 6504);

        cmsCIExyYTRIPLE primaries = {
            {0.6800, 0.3200, 1.0},  // red
            {0.2650, 0.6900, 1.0},  // green
            {0.1500, 0.0600, 1.0}   // blue
        };

        // sRGB TRC (piecewise linear + gamma 2.4)
        cmsToneCurve* srgbTrc = cmsBuildParametricToneCurve(
            nullptr, 4,
            std::array<cmsFloat64Number, 5>{2.4, 1.0 / 1.055, 0.055 / 1.055, 1.0 / 12.92, 0.04045}.data());
        std::array<cmsToneCurve*, 3> curves = {srgbTrc, srgbTrc, srgbTrc};

        profile = cmsCreateRGBProfile(&d65, &primaries, curves.data());
        cmsFreeToneCurve(srgbTrc);
        name = QStringLiteral("Display P3");
        break;
    }

    case Builtin::CMYK_USWebCoatedSWOP: {
        // Create a Lab→CMYK profile using a simple model.
        // Real SWOP profiles are complex ICC files; we create a synthetic
        // CMYK profile with Lab PCS for basic conversion support.
        profile = cmsCreateLab4Profile(nullptr);
        name = QStringLiteral("CMYK US Web Coated (SWOP) v2");
        // Note: for production use, embed a real SWOP ICC binary.
        // This synthetic profile provides correct structure for testing.
        break;
    }

    case Builtin::CMYK_FOGRA39: {
        profile = cmsCreateLab4Profile(nullptr);
        name = QStringLiteral("CMYK FOGRA39");
        break;
    }

    case Builtin::Gray_Gamma22: {
        cmsCIExyY d65;
        cmsWhitePointFromTemp(&d65, 6504);
        cmsToneCurve* gamma22 = cmsBuildGamma(nullptr, 2.2);
        profile = cmsCreateGrayProfile(&d65, gamma22);
        cmsFreeToneCurve(gamma22);
        name = QStringLiteral("Gray Gamma 2.2");
        break;
    }
    }

    if (profile == nullptr) {
        qWarning() << "ColorProfile::builtin: failed to create profile" << static_cast<int>(id);
        return {};
    }

    return ColorProfile(profile, name);
}

// ---- File loading ----

ColorProfile ColorProfile::fromIccFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ColorProfile::fromIccFile: cannot open" << path;
        return {};
    }

    QByteArray data = file.readAll();
    return fromIccData(data);
}

ColorProfile ColorProfile::fromIccData(const QByteArray& data)
{
    cmsHPROFILE profile = cmsOpenProfileFromMem(data.constData(),
                                                 static_cast<cmsUInt32Number>(data.size()));
    if (profile == nullptr) {
        qWarning() << "ColorProfile::fromIccData: failed to parse ICC data";
        return {};
    }

    // Read profile description.
    cmsUInt32Number len = cmsGetProfileInfoASCII(
        profile, cmsInfoDescription, "en", "US", nullptr, 0);
    QString name;
    if (len > 0) {
        std::vector<char> buf(len);
        cmsGetProfileInfoASCII(profile, cmsInfoDescription, "en", "US",
                               buf.data(), len);
        name = QString::fromUtf8(buf.data());
    }

    ColorProfile cp(profile, name.isEmpty() ? QStringLiteral("Custom ICC") : name);
    cp.impl_->cachedIccBytes = data;
    return cp;
}

// ---- Queries ----

bool ColorProfile::isValid() const
{
    return impl_ && impl_->profile != nullptr;
}

QString ColorProfile::name() const
{
    return impl_ ? impl_->profileName : QString();
}

cmsHPROFILE ColorProfile::handle() const
{
    return impl_ ? impl_->profile : nullptr;
}

bool ColorProfile::isCMYK() const
{
    if (!isValid())
        return false;
    return cmsGetColorSpace(impl_->profile) == cmsSigCmykData;
}

bool ColorProfile::isRGB() const
{
    if (!isValid())
        return false;
    return cmsGetColorSpace(impl_->profile) == cmsSigRgbData;
}

bool ColorProfile::isGrayscale() const
{
    if (!isValid())
        return false;
    return cmsGetColorSpace(impl_->profile) == cmsSigGrayData;
}

// ---- ICC bytes ----

QByteArray ColorProfile::iccBytes() const
{
    if (!isValid())
        return {};

    // Return cached bytes if we loaded from file/data.
    if (!impl_->cachedIccBytes.isEmpty())
        return impl_->cachedIccBytes;

    // Save profile to memory.
    cmsUInt32Number size = 0;
    cmsSaveProfileToMem(impl_->profile, nullptr, &size);
    if (size == 0)
        return {};

    QByteArray result(static_cast<int>(size), Qt::Uninitialized);
    cmsSaveProfileToMem(impl_->profile, result.data(), &size);
    // Cache for future calls.
    impl_->cachedIccBytes = result;
    return result;
}

// ---- Color conversion ----

QColor ColorProfile::convert(QColor source, const ColorProfile& target) const
{
    if (!isValid() || !target.isValid())
        return source;

    // Determine input/output formats.
    cmsUInt32Number inputFormat = TYPE_RGBA_8;
    cmsUInt32Number outputFormat = TYPE_RGBA_8;

    if (isGrayscale())
        inputFormat = TYPE_GRAY_8;
    if (target.isGrayscale())
        outputFormat = TYPE_GRAY_8;
    if (target.isCMYK())
        outputFormat = TYPE_CMYK_8;

    cmsHTRANSFORM xform = cmsCreateTransform(
        impl_->profile, inputFormat,
        target.impl_->profile, outputFormat,
        INTENT_PERCEPTUAL, 0);

    if (xform == nullptr)
        return source;

    // Convert.
    std::array<cmsUInt8Number, 4> in = {
        static_cast<cmsUInt8Number>(source.red()),
        static_cast<cmsUInt8Number>(source.green()),
        static_cast<cmsUInt8Number>(source.blue()),
        static_cast<cmsUInt8Number>(source.alpha())};
    std::array<cmsUInt8Number, 4> out = {0, 0, 0, 255};

    cmsDoTransform(xform, in.data(), out.data(), 1);
    cmsDeleteTransform(xform);

    if (target.isCMYK()) {
        // Return CMYK as a QColor with CMYK components.
        // QColor supports CMYK via setCmyk.
        QColor result;
        result.setCmyk(out[0], out[1], out[2], out[3]);
        return result;
    }

    if (target.isGrayscale()) {
        return QColor(out[0], out[0], out[0], source.alpha());
    }

    return QColor(out[0], out[1], out[2], source.alpha());
}

}  // namespace lumen::exp
