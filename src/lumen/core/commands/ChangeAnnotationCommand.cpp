#include "ChangeAnnotationCommand.h"

#include "plot/Annotation.h"
#include "plot/AnnotationLayer.h"
#include "plot/PlotScene.h"

#include <cassert>

namespace lumen::core::commands {

ChangeAnnotationCommand::ChangeAnnotationCommand(
    plot::PlotScene* scene, int annotationId, QJsonObject newState)
    : scene_(scene)
    , annotationId_(annotationId)
    , newState_(std::move(newState))
{
    auto* layer = scene_->annotationLayer();
    assert(layer && "ChangeAnnotationCommand: scene has no annotation layer");
    auto* ann = layer->find(annotationId_);
    assert(ann && "ChangeAnnotationCommand: annotation not found");
    oldState_ = ann->toJson();
}

void ChangeAnnotationCommand::execute()
{
    applyState(newState_);
}

void ChangeAnnotationCommand::undo()
{
    applyState(oldState_);
}

QString ChangeAnnotationCommand::description() const
{
    return QStringLiteral("Change annotation %1").arg(annotationId_);
}

void ChangeAnnotationCommand::applyState(const QJsonObject& state)
{
    auto* layer = scene_->annotationLayer();
    assert(layer);

    // Remove old annotation and replace with deserialized new state.
    layer->removeAnnotation(annotationId_);

    auto newAnn = plot::Annotation::fromJson(state);
    if (newAnn) {
        newAnn->setId(annotationId_);
        layer->addAnnotation(std::move(newAnn));
    }
}

}  // namespace lumen::core::commands
