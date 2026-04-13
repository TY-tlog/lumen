#include "AnnotationPropertyDialog.h"

#include "core/CommandBus.h"
#include "core/commands/ChangeAnnotationCommand.h"
#include "plot/PlotScene.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace lumen::ui {

AnnotationPropertyDialog::AnnotationPropertyDialog(
    plot::Annotation* annotation, plot::PlotScene* scene,
    core::CommandBus* bus, QWidget* parent)
    : QDialog(parent)
    , annotation_(annotation)
    , scene_(scene)
    , bus_(bus)
{
    setWindowTitle(tr("Annotation Properties"));
    setMinimumWidth(300);

    auto* layout = new QVBoxLayout(this);

    // Type label.
    QString typeName;
    switch (annotation->type()) {
    case plot::Annotation::Type::Arrow: typeName = tr("Arrow"); break;
    case plot::Annotation::Type::Box: typeName = tr("Box"); break;
    case plot::Annotation::Type::Callout: typeName = tr("Callout"); break;
    case plot::Annotation::Type::Text: typeName = tr("Text"); break;
    case plot::Annotation::Type::ScaleBar: typeName = tr("Scale Bar"); break;
    case plot::Annotation::Type::ColorBar: typeName = tr("Color Bar"); break;
    }
    layout->addWidget(new QLabel(tr("Type: %1").arg(typeName), this));

    // Placeholder for type-specific controls.
    // Phase 9 provides the dialog structure; type-specific editors
    // are added as the annotation system matures.
    layout->addWidget(new QLabel(tr("(Edit properties via JSON in workspace file)"), this));

    buttonBox_ = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &AnnotationPropertyDialog::onAccepted);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox_);
}

void AnnotationPropertyDialog::onAccepted()
{
    // For now, accept without changes. Full per-type editors in Phase 9 continuation.
    accept();
}

}  // namespace lumen::ui
