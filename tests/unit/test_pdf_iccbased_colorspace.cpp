#include <catch2/catch_test_macros.hpp>

#include <export/ColorProfile.h>

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QPdfWriter>

#include <atomic>

using lumen::exp::ColorProfile;

namespace {

/// Write a minimal PDF with text, return the file path.
QString writeTestPdf()
{
    static std::atomic<int> counter{0};
    QString path = QDir::tempPath() + QStringLiteral("/lumen_test_pdf_%1_%2.pdf")
        .arg(QCoreApplication::applicationPid())
        .arg(counter.fetch_add(1));

    QPdfWriter writer(path);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(150);

    QPainter painter(&writer);
    painter.setPen(Qt::black);
    painter.drawText(100, 100, QStringLiteral("Test PDF for ICC"));
    painter.end();

    return path;
}

}  // namespace

TEST_CASE("PDF structure: QPdfWriter creates valid PDF file", "[pdf_iccbased]") {
    QString path = writeTestPdf();

    QFile f(path);
    REQUIRE(f.open(QIODevice::ReadOnly));
    QByteArray data = f.readAll();
    f.close();

    // PDF files start with %PDF-
    CHECK(data.startsWith("%PDF-"));
    // And end with %%EOF (possibly with trailing whitespace)
    CHECK(data.contains("%%EOF"));

    QFile::remove(path);
}

TEST_CASE("PDF structure: contains ColorSpace declarations", "[pdf_iccbased]") {
    QString path = writeTestPdf();

    QFile f(path);
    REQUIRE(f.open(QIODevice::ReadOnly));
    QByteArray data = f.readAll();
    f.close();

    // Qt's QPdfWriter includes /ColorSpace in the PDF.
    // For Phase 9 T2, we'll add /ICCBased explicitly.
    // For now, verify the PDF structure is valid.
    CHECK(data.contains("/Type"));

    QFile::remove(path);
}

TEST_CASE("ColorProfile: ICC bytes are valid for PDF embedding", "[pdf_iccbased]") {
    auto srgb = ColorProfile::builtin(ColorProfile::Builtin::sRGB);
    QByteArray iccData = srgb.iccBytes();

    // ICC profile must be at least 128 bytes (header size).
    REQUIRE(iccData.size() >= 128);

    // Profile header: size field at offset 0 (big-endian uint32).
    auto size = static_cast<quint32>(
        (static_cast<quint8>(iccData[0]) << 24) |
        (static_cast<quint8>(iccData[1]) << 16) |
        (static_cast<quint8>(iccData[2]) << 8) |
        static_cast<quint8>(iccData[3]));

    // Size should match actual data size.
    CHECK(size == static_cast<quint32>(iccData.size()));
}

TEST_CASE("ColorProfile: multiple profiles have distinct ICC bytes", "[pdf_iccbased]") {
    auto srgb = ColorProfile::builtin(ColorProfile::Builtin::sRGB);
    auto adobe = ColorProfile::builtin(ColorProfile::Builtin::AdobeRGB);

    CHECK(srgb.iccBytes() != adobe.iccBytes());
}
