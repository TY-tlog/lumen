#include "AnnotationToolbar.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QToolButton>

namespace lumen::ui {

AnnotationToolbar::AnnotationToolbar(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(2);

    group_ = new QButtonGroup(this);
    group_->setExclusive(false);

    struct ButtonDef {
        QString text;
        plot::Annotation::Type type;
    };

    static const ButtonDef buttons[] = {
        {QStringLiteral("Arrow"), plot::Annotation::Type::Arrow},
        {QStringLiteral("Box"), plot::Annotation::Type::Box},
        {QStringLiteral("Callout"), plot::Annotation::Type::Callout},
        {QStringLiteral("Text"), plot::Annotation::Type::Text},
        {QStringLiteral("Scale"), plot::Annotation::Type::ScaleBar},
        {QStringLiteral("Color"), plot::Annotation::Type::ColorBar},
    };

    int id = 0;
    for (const auto& def : buttons) {
        auto* btn = new QToolButton(this);
        btn->setText(def.text);
        btn->setCheckable(true);
        btn->setMinimumWidth(48);
        group_->addButton(btn, id);
        layout->addWidget(btn);
        ++id;
    }

    layout->addStretch();

    connect(group_, &QButtonGroup::idToggled,
            this, &AnnotationToolbar::onButtonToggled);
}

int AnnotationToolbar::buttonCount() const
{
    return static_cast<int>(group_->buttons().size());
}

void AnnotationToolbar::onButtonToggled(int id, bool checked)
{
    static const plot::Annotation::Type types[] = {
        plot::Annotation::Type::Arrow,
        plot::Annotation::Type::Box,
        plot::Annotation::Type::Callout,
        plot::Annotation::Type::Text,
        plot::Annotation::Type::ScaleBar,
        plot::Annotation::Type::ColorBar,
    };

    if (checked) {
        // Uncheck other buttons.
        for (auto* btn : group_->buttons()) {
            if (group_->id(btn) != id)
                btn->setChecked(false);
        }
        if (id >= 0 && id < 6)
            emit toolSelected(types[id]);
    } else {
        emit toolDeselected();
    }
}

}  // namespace lumen::ui
