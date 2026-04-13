#include <catch2/catch_test_macros.hpp>

#include "plot/AnnotationLayer.h"
#include "plot/CoordinateMapper.h"
#include "plot/annotations/ArrowAnnotation.h"
#include "plot/annotations/BoxAnnotation.h"
#include "plot/annotations/TextAnnotation.h"

#include <QJsonArray>
#include <QPointF>
#include <QRectF>

#include <memory>

using lumen::plot::Annotation;
using lumen::plot::AnnotationLayer;
using lumen::plot::ArrowAnnotation;
using lumen::plot::BoxAnnotation;
using lumen::plot::CoordinateMapper;
using lumen::plot::TextAnnotation;

namespace {

const QRectF kPlotArea(100.0, 50.0, 400.0, 400.0);

CoordinateMapper makeMapper()
{
    return CoordinateMapper(0.0, 10.0, 0.0, 10.0, kPlotArea);
}

}  // namespace

TEST_CASE("AnnotationLayer: add, remove, and count",
          "[annotation][annotation_layer]")
{
    AnnotationLayer layer;
    REQUIRE(layer.count() == 0);

    const int id1 = layer.addAnnotation(
        std::make_unique<ArrowAnnotation>(QPointF(1.0, 2.0), QPointF(3.0, 4.0)));
    REQUIRE(layer.count() == 1);

    const int id2 = layer.addAnnotation(
        std::make_unique<TextAnnotation>("label", QPointF(5.0, 5.0)));
    REQUIRE(layer.count() == 2);
    REQUIRE(id1 != id2);

    REQUIRE(layer.removeAnnotation(id1));
    REQUIRE(layer.count() == 1);

    // Removing the same ID again returns false.
    REQUIRE_FALSE(layer.removeAnnotation(id1));
    REQUIRE(layer.count() == 1);
}

TEST_CASE("AnnotationLayer: find by ID", "[annotation][annotation_layer]")
{
    AnnotationLayer layer;
    const int id = layer.addAnnotation(
        std::make_unique<BoxAnnotation>(QPointF(0.0, 0.0), QPointF(5.0, 5.0)));

    Annotation* found = layer.find(id);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == Annotation::Type::Box);
    REQUIRE(found->id() == id);

    // Non-existent ID returns nullptr.
    REQUIRE(layer.find(id + 999) == nullptr);
}

TEST_CASE("AnnotationLayer: hitTest returns top-most annotation",
          "[annotation][annotation_layer]")
{
    AnnotationLayer layer;
    const auto mapper = makeMapper();

    // Two overlapping boxes in data space (0..5 covers a chunk of the plot).
    layer.addAnnotation(
        std::make_unique<BoxAnnotation>(QPointF(0.0, 0.0), QPointF(5.0, 5.0)));
    const int topId = layer.addAnnotation(
        std::make_unique<BoxAnnotation>(QPointF(0.0, 0.0), QPointF(5.0, 5.0)));

    // Point clearly inside both boxes: data (2.5, 2.5) -> pixel.
    const QPointF pixel = mapper.dataToPixel(2.5, 2.5);
    const auto hit = layer.hitTest(pixel.toPoint(), mapper, kPlotArea);

    REQUIRE(hit.has_value());
    // The top-most (last added) annotation should be returned.
    CHECK(hit->annotationId == topId);

    // Point well outside the plot area should miss.
    const auto miss = layer.hitTest(QPoint(-100, -100), mapper, kPlotArea);
    REQUIRE_FALSE(miss.has_value());
}

TEST_CASE("AnnotationLayer: clear removes all annotations",
          "[annotation][annotation_layer]")
{
    AnnotationLayer layer;
    layer.addAnnotation(
        std::make_unique<ArrowAnnotation>(QPointF(0.0, 0.0), QPointF(1.0, 1.0)));
    layer.addAnnotation(
        std::make_unique<TextAnnotation>("hello", QPointF(2.0, 3.0)));
    REQUIRE(layer.count() == 2);

    layer.clear();
    REQUIRE(layer.count() == 0);
    REQUIRE(layer.all().isEmpty());
}

TEST_CASE("AnnotationLayer: toJsonArray / fromJsonArray roundtrip",
          "[annotation][annotation_layer]")
{
    AnnotationLayer original;
    original.addAnnotation(
        std::make_unique<ArrowAnnotation>(QPointF(1.0, 2.0), QPointF(3.0, 4.0)));
    original.addAnnotation(
        std::make_unique<TextAnnotation>("note", QPointF(5.0, 6.0)));
    original.addAnnotation(
        std::make_unique<BoxAnnotation>(QPointF(0.0, 0.0), QPointF(7.0, 8.0)));

    const QJsonArray arr = original.toJsonArray();
    REQUIRE(arr.size() == 3);

    AnnotationLayer restored;
    restored.fromJsonArray(arr);
    REQUIRE(restored.count() == 3);

    // Verify type order is preserved.
    const auto all = restored.all();
    CHECK(all[0]->type() == Annotation::Type::Arrow);
    CHECK(all[1]->type() == Annotation::Type::Text);
    CHECK(all[2]->type() == Annotation::Type::Box);
}
