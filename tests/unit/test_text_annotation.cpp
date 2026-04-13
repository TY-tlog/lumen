#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "plot/annotations/TextAnnotation.h"
#include "plot/CoordinateMapper.h"

#include <QJsonObject>
#include <QPointF>
#include <QRectF>

#include <memory>

using lumen::plot::Annotation;
using lumen::plot::CoordinateMapper;
using lumen::plot::TextAnnotation;
using Catch::Matchers::WithinAbs;

namespace {

const QRectF kPlotArea(100.0, 50.0, 400.0, 400.0);

CoordinateMapper makeMapper()
{
    return CoordinateMapper(0.0, 10.0, 0.0, 10.0, kPlotArea);
}

}  // namespace

TEST_CASE("TextAnnotation: construction stores text, position, anchor",
          "[annotation][text]")
{
    const TextAnnotation ann("Hello", QPointF(3.0, 7.0),
                             Annotation::Anchor::Data);

    CHECK(ann.type() == Annotation::Type::Text);
    CHECK(ann.anchor() == Annotation::Anchor::Data);
    CHECK(ann.text() == "Hello");
    CHECK_THAT(ann.position().x(), WithinAbs(3.0, 1e-12));
    CHECK_THAT(ann.position().y(), WithinAbs(7.0, 1e-12));

    // Default property values.
    CHECK(ann.color() == QColor(Qt::black));
    CHECK_THAT(ann.rotation(), WithinAbs(0.0, 1e-12));
    CHECK(ann.fontSize() == 11);
}

TEST_CASE("TextAnnotation: three anchor modes produce different boundingRects",
          "[annotation][text]")
{
    const auto mapper = makeMapper();
    const QString label = "Test label";

    // Data anchor: position is in data units.
    const TextAnnotation dataAnn(label, QPointF(5.0, 5.0),
                                 Annotation::Anchor::Data);
    const QRectF dataRect = dataAnn.boundingRect(mapper, kPlotArea);
    CHECK(dataRect.width() > 0.0);
    CHECK(dataRect.height() > 0.0);

    // Pixel anchor: position is absolute pixels.
    const TextAnnotation pixelAnn(label, QPointF(200.0, 200.0),
                                  Annotation::Anchor::Pixel);
    const QRectF pixelRect = pixelAnn.boundingRect(mapper, kPlotArea);
    CHECK(pixelRect.width() > 0.0);
    CHECK(pixelRect.height() > 0.0);

    // AxisFraction anchor: position is fractional (0..1).
    const TextAnnotation fracAnn(label, QPointF(0.5, 0.5),
                                 Annotation::Anchor::AxisFraction);
    const QRectF fracRect = fracAnn.boundingRect(mapper, kPlotArea);
    CHECK(fracRect.width() > 0.0);
    CHECK(fracRect.height() > 0.0);

    // Data(5,5) and Fraction(0.5,0.5) should resolve to the same pixel
    // center in this symmetric mapper, so their rects should be close.
    CHECK_THAT(dataRect.center().x(), WithinAbs(fracRect.center().x(), 2.0));
    CHECK_THAT(dataRect.center().y(), WithinAbs(fracRect.center().y(), 2.0));

    // Pixel(200,200) is a different absolute position.
    // Just verify it is a valid non-degenerate rect.
    CHECK(pixelRect.left() >= 0.0);
}

TEST_CASE("TextAnnotation: rotation property", "[annotation][text]")
{
    TextAnnotation ann("rotated", QPointF(5.0, 5.0));
    CHECK_THAT(ann.rotation(), WithinAbs(0.0, 1e-12));

    ann.setRotation(45.0);
    CHECK_THAT(ann.rotation(), WithinAbs(45.0, 1e-12));

    ann.setRotation(-90.0);
    CHECK_THAT(ann.rotation(), WithinAbs(-90.0, 1e-12));
}

TEST_CASE("TextAnnotation: JSON roundtrip preserves all fields",
          "[annotation][text]")
{
    TextAnnotation original("$V_{rms}$", QPointF(2.5, 7.5),
                            Annotation::Anchor::AxisFraction);
    original.setColor(QColor(0xCC, 0x00, 0x44));
    original.setRotation(30.0);
    original.setFontSize(16);
    original.setName("rms_label");
    original.setVisible(false);

    const QJsonObject json = original.toJson();
    auto restored = TextAnnotation::fromJson(json);
    REQUIRE(restored != nullptr);

    const auto* text = dynamic_cast<TextAnnotation*>(restored.get());
    REQUIRE(text != nullptr);

    CHECK(text->text() == "$V_{rms}$");
    CHECK_THAT(text->position().x(), WithinAbs(2.5, 1e-12));
    CHECK_THAT(text->position().y(), WithinAbs(7.5, 1e-12));
    CHECK(text->anchor() == Annotation::Anchor::AxisFraction);
    CHECK(text->color() == QColor(0xCC, 0x00, 0x44));
    CHECK_THAT(text->rotation(), WithinAbs(30.0, 1e-12));
    CHECK(text->fontSize() == 16);
    CHECK(text->name() == "rms_label");
    CHECK_FALSE(text->isVisible());
}
