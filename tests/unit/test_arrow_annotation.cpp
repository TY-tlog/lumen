#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "plot/annotations/ArrowAnnotation.h"
#include "plot/CoordinateMapper.h"

#include <QJsonObject>
#include <QPointF>
#include <QRectF>

#include <memory>

using lumen::plot::Annotation;
using lumen::plot::ArrowAnnotation;
using lumen::plot::CoordinateMapper;
using Catch::Matchers::WithinAbs;

namespace {

const QRectF kPlotArea(100.0, 50.0, 400.0, 400.0);

CoordinateMapper makeMapper()
{
    return CoordinateMapper(0.0, 10.0, 0.0, 10.0, kPlotArea);
}

}  // namespace

TEST_CASE("ArrowAnnotation: construction stores from/to and anchor",
          "[annotation][arrow]")
{
    const ArrowAnnotation arrow(QPointF(1.0, 2.0), QPointF(8.0, 9.0),
                                Annotation::Anchor::Data);

    CHECK(arrow.type() == Annotation::Type::Arrow);
    CHECK(arrow.anchor() == Annotation::Anchor::Data);
    CHECK_THAT(arrow.from().x(), WithinAbs(1.0, 1e-12));
    CHECK_THAT(arrow.from().y(), WithinAbs(2.0, 1e-12));
    CHECK_THAT(arrow.to().x(), WithinAbs(8.0, 1e-12));
    CHECK_THAT(arrow.to().y(), WithinAbs(9.0, 1e-12));
}

TEST_CASE("ArrowAnnotation: color, lineWidth, headStyle properties",
          "[annotation][arrow]")
{
    ArrowAnnotation arrow(QPointF(0.0, 0.0), QPointF(5.0, 5.0));

    // Check defaults.
    CHECK(arrow.color() == Qt::red);
    CHECK_THAT(arrow.lineWidth(), WithinAbs(1.5, 1e-12));
    CHECK(arrow.headStyle() == ArrowAnnotation::HeadStyle::Filled);

    // Mutate and verify.
    arrow.setColor(Qt::blue);
    CHECK(arrow.color() == QColor(Qt::blue));

    arrow.setLineWidth(3.0);
    CHECK_THAT(arrow.lineWidth(), WithinAbs(3.0, 1e-12));

    arrow.setHeadStyle(ArrowAnnotation::HeadStyle::Double);
    CHECK(arrow.headStyle() == ArrowAnnotation::HeadStyle::Double);
}

TEST_CASE("ArrowAnnotation: JSON roundtrip preserves all fields",
          "[annotation][arrow]")
{
    ArrowAnnotation original(QPointF(1.5, 2.5), QPointF(7.5, 8.5),
                             Annotation::Anchor::Pixel);
    original.setColor(QColor(0x33, 0x66, 0x99));
    original.setLineWidth(2.5);
    original.setHeadStyle(ArrowAnnotation::HeadStyle::Open);
    original.setName("my arrow");
    original.setVisible(false);

    const QJsonObject json = original.toJson();
    auto restored = ArrowAnnotation::fromJson(json);
    REQUIRE(restored != nullptr);

    const auto* arrow = dynamic_cast<ArrowAnnotation*>(restored.get());
    REQUIRE(arrow != nullptr);

    CHECK_THAT(arrow->from().x(), WithinAbs(1.5, 1e-12));
    CHECK_THAT(arrow->from().y(), WithinAbs(2.5, 1e-12));
    CHECK_THAT(arrow->to().x(), WithinAbs(7.5, 1e-12));
    CHECK_THAT(arrow->to().y(), WithinAbs(8.5, 1e-12));
    CHECK(arrow->color() == QColor(0x33, 0x66, 0x99));
    CHECK_THAT(arrow->lineWidth(), WithinAbs(2.5, 1e-12));
    CHECK(arrow->headStyle() == ArrowAnnotation::HeadStyle::Open);
    CHECK(arrow->name() == "my arrow");
    CHECK_FALSE(arrow->isVisible());
}

TEST_CASE("ArrowAnnotation: boundingRect is non-empty",
          "[annotation][arrow]")
{
    const ArrowAnnotation arrow(QPointF(2.0, 3.0), QPointF(8.0, 7.0));
    const auto mapper = makeMapper();

    const QRectF rect = arrow.boundingRect(mapper, kPlotArea);
    CHECK(rect.width() > 0.0);
    CHECK(rect.height() > 0.0);

    // The rect should contain the pixel mappings of both endpoints.
    const QPointF fromPx = mapper.dataToPixel(2.0, 3.0);
    const QPointF toPx = mapper.dataToPixel(8.0, 7.0);
    CHECK(rect.contains(fromPx));
    CHECK(rect.contains(toPx));
}
