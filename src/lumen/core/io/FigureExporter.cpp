#include "core/io/FigureExporter.h"

#include "plot/PlotRenderer.h"
#include "plot/PlotScene.h"

#include <QImage>
#include <QPainter>
#include <QPdfWriter>
#include <QSvgGenerator>

namespace lumen::core::io {

QString FigureExporter::exportFigure(const plot::PlotScene* scene,
                                     const Options& opts) {
    if (!scene) {
        return QStringLiteral("PlotScene is null");
    }
    if (opts.outputPath.isEmpty()) {
        return QStringLiteral("Output path is empty");
    }

    switch (opts.format) {
        case Format::Png:
            return exportPng(scene, opts);
        case Format::Svg:
            return exportSvg(scene, opts);
        case Format::Pdf:
            return exportPdf(scene, opts);
    }
    return QStringLiteral("Unknown export format");
}

QString FigureExporter::exportPng(const plot::PlotScene* scene,
                                  const Options& opts) {
    QImage image(opts.widthPx, opts.heightPx, QImage::Format_ARGB32);

    if (opts.transparentBackground) {
        image.fill(Qt::transparent);
    } else {
        image.fill(Qt::white);
    }

    // Set DPI metadata.
    const double dotsPerMeter = opts.dpi / 0.0254;
    image.setDotsPerMeterX(static_cast<int>(dotsPerMeter));
    image.setDotsPerMeterY(static_cast<int>(dotsPerMeter));

    QPainter painter(&image);
    if (!painter.isActive()) {
        return QStringLiteral("Failed to create QPainter for PNG image");
    }

    plot::PlotRenderer renderer;
    renderer.render(painter, *scene, QSizeF(opts.widthPx, opts.heightPx));
    painter.end();

    // For transparent background: the renderer fills white background,
    // so replace the background fill with transparency by compositing.
    // Since PlotRenderer fills background::primary (white), we re-render
    // on transparent by rendering content over a transparent base.
    if (opts.transparentBackground) {
        // Create transparent base.
        QImage transparentImage(opts.widthPx, opts.heightPx, QImage::Format_ARGB32);
        transparentImage.fill(Qt::transparent);

        // Render a white reference to detect background pixels.
        // Pixel-by-pixel: if the rendered pixel matches the background
        // color from the opaque render (white), make it transparent.
        QImage opaqueImage(opts.widthPx, opts.heightPx, QImage::Format_ARGB32);
        opaqueImage.fill(Qt::white);
        QPainter opaquePainter(&opaqueImage);
        renderer.render(opaquePainter, *scene, QSizeF(opts.widthPx, opts.heightPx));
        opaquePainter.end();

        // Copy non-background pixels to transparent image.
        const QColor bgColor(255, 255, 255);
        for (int py = 0; py < opts.heightPx; ++py) {
            for (int px = 0; px < opts.widthPx; ++px) {
                QColor pixel = opaqueImage.pixelColor(px, py);
                if (pixel != bgColor) {
                    transparentImage.setPixelColor(px, py, pixel);
                }
            }
        }

        transparentImage.setDotsPerMeterX(static_cast<int>(dotsPerMeter));
        transparentImage.setDotsPerMeterY(static_cast<int>(dotsPerMeter));

        if (!transparentImage.save(opts.outputPath, "PNG")) {
            return QString("Failed to save PNG to %1").arg(opts.outputPath);
        }
        return {};
    }

    if (!image.save(opts.outputPath, "PNG")) {
        return QString("Failed to save PNG to %1").arg(opts.outputPath);
    }
    return {};
}

QString FigureExporter::exportSvg(const plot::PlotScene* scene,
                                  const Options& opts) {
    QSvgGenerator generator;
    generator.setFileName(opts.outputPath);
    generator.setSize(QSize(opts.widthPx, opts.heightPx));
    generator.setViewBox(QRect(0, 0, opts.widthPx, opts.heightPx));
    generator.setTitle(QStringLiteral("Lumen Figure"));

    QPainter painter(&generator);
    if (!painter.isActive()) {
        return QStringLiteral("Failed to create QPainter for SVG generator");
    }

    plot::PlotRenderer renderer;
    renderer.render(painter, *scene, QSizeF(opts.widthPx, opts.heightPx));
    painter.end();

    return {};
}

QString FigureExporter::exportPdf(const plot::PlotScene* scene,
                                  const Options& opts) {
    QPdfWriter writer(opts.outputPath);
    writer.setPageSize(
        QPageSize(QSizeF(opts.widthPx, opts.heightPx), QPageSize::Point));
    writer.setResolution(opts.dpi);

    QPainter painter(&writer);
    if (!painter.isActive()) {
        return QString("Failed to create QPainter for PDF at %1").arg(opts.outputPath);
    }

    plot::PlotRenderer renderer;
    // QPdfWriter coordinates are in device pixels at the writer's DPI.
    // Scale the painter so PlotRenderer draws at the logical (point) size,
    // which keeps text and margins proportional to the output page.
    const QRectF paintRect =
        writer.pageLayout().paintRectPixels(writer.resolution());
    double scaleX = paintRect.width() / opts.widthPx;
    double scaleY = paintRect.height() / opts.heightPx;
    painter.scale(scaleX, scaleY);
    renderer.render(painter, *scene,
                    QSizeF(opts.widthPx, opts.heightPx));
    painter.end();

    return {};
}

}  // namespace lumen::core::io
