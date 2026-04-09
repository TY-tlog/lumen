#include "ChangeTitleCommand.h"

#include "plot/PlotScene.h"

namespace lumen::core::commands {

ChangeTitleCommand::ChangeTitleCommand(plot::PlotScene* scene, QString newTitle,
                                       int newFontPx, QFont::Weight newWeight)
    : scene_(scene)
    , newTitle_(std::move(newTitle))
    , newFontPx_(newFontPx)
    , newWeight_(newWeight) {
    // Capture old values from the current state of the scene.
    oldTitle_ = scene_->title();
    oldFontPx_ = scene_->titleFontPx();
    oldWeight_ = scene_->titleWeight();
}

void ChangeTitleCommand::execute() {
    scene_->setTitle(newTitle_);
    scene_->setTitleFontPx(newFontPx_);
    scene_->setTitleWeight(newWeight_);
}

void ChangeTitleCommand::undo() {
    scene_->setTitle(oldTitle_);
    scene_->setTitleFontPx(oldFontPx_);
    scene_->setTitleWeight(oldWeight_);
}

QString ChangeTitleCommand::description() const {
    return QStringLiteral("Change plot title");
}

}  // namespace lumen::core::commands
