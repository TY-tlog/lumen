#include "Scene3D.h"

#include <algorithm>

namespace lumen::plot3d {

void Scene3D::addItem(std::unique_ptr<PlotItem3D> item)
{
    items_.push_back(std::move(item));
}

void Scene3D::removeItem(int index)
{
    if (index >= 0 && index < static_cast<int>(items_.size())) {
        items_.erase(items_.begin() + index);
    }
}

const std::vector<std::unique_ptr<PlotItem3D>>& Scene3D::items() const
{
    return items_;
}

PlotItem3D* Scene3D::itemAt(int index)
{
    if (index >= 0 && index < static_cast<int>(items_.size()))
        return items_[static_cast<std::size_t>(index)].get();
    return nullptr;
}

int Scene3D::itemCount() const
{
    return static_cast<int>(items_.size());
}

void Scene3D::clearItems()
{
    items_.clear();
}

std::vector<Light>& Scene3D::lights()
{
    return lights_;
}

const std::vector<Light>& Scene3D::lights() const
{
    return lights_;
}

BoundingBox3D Scene3D::sceneBounds() const
{
    BoundingBox3D bounds;
    bool first = true;

    for (const auto& item : items_) {
        if (!item || !item->isVisible())
            continue;
        BoundingBox3D itemBounds = item->dataBounds();
        if (!itemBounds.isValid())
            continue;
        if (first) {
            bounds = itemBounds;
            first = false;
        } else {
            bounds = bounds.united(itemBounds);
        }
    }

    return bounds;
}

void Scene3D::addDefaultLights()
{
    // One directional light.
    Light directional;
    directional.type = LightType::Directional;
    directional.direction = QVector3D(0.5f, -1.0f, -0.5f).normalized();
    directional.color = QVector3D(1.0f, 1.0f, 1.0f);
    directional.intensity = 1.0f;
    lights_.push_back(directional);

    // One ambient light.
    Light ambient;
    ambient.type = LightType::Ambient;
    ambient.color = QVector3D(0.2f, 0.2f, 0.2f);
    ambient.intensity = 1.0f;
    lights_.push_back(ambient);
}

}  // namespace lumen::plot3d
