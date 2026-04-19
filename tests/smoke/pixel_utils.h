#pragma once

#include <QImage>

#include <cmath>

namespace lumen::test {

inline bool hasVisualContent(const QImage& fb, double minVariance = 10.0)
{
    if (fb.isNull() || fb.width() == 0 || fb.height() == 0)
        return false;

    double sum = 0.0;
    double sumSq = 0.0;
    int count = 0;

    for (int y = 0; y < fb.height(); ++y) {
        for (int x = 0; x < fb.width(); ++x) {
            QColor c = fb.pixelColor(x, y);
            double gray = 0.299 * c.redF() + 0.587 * c.greenF() + 0.114 * c.blueF();
            sum += gray;
            sumSq += gray * gray;
            ++count;
        }
    }

    double mean = sum / count;
    double variance = sumSq / count - mean * mean;
    double stddev = std::sqrt(std::max(0.0, variance)) * 255.0;
    return stddev > minVariance;
}

inline bool isAllBlack(const QImage& fb)
{
    if (fb.isNull())
        return true;
    for (int y = 0; y < fb.height(); ++y) {
        for (int x = 0; x < fb.width(); ++x) {
            QRgb px = fb.pixel(x, y);
            if (qRed(px) > 5 || qGreen(px) > 5 || qBlue(px) > 5)
                return false;
        }
    }
    return true;
}

inline bool isAllWhite(const QImage& fb)
{
    if (fb.isNull())
        return true;
    for (int y = 0; y < fb.height(); ++y) {
        for (int x = 0; x < fb.width(); ++x) {
            QRgb px = fb.pixel(x, y);
            if (qRed(px) < 250 || qGreen(px) < 250 || qBlue(px) < 250)
                return false;
        }
    }
    return true;
}

}  // namespace lumen::test
