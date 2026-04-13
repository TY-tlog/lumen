#pragma once

#include "BoundingBox3D.h"
#include "Light.h"
#include "PlotItem3D.h"

#include <memory>
#include <vector>

namespace lumen::plot3d {

class Scene3D {
public:
    void addItem(std::unique_ptr<PlotItem3D> item);
    void removeItem(int index);
    [[nodiscard]] const std::vector<std::unique_ptr<PlotItem3D>>& items() const;
    [[nodiscard]] PlotItem3D* itemAt(int index);
    [[nodiscard]] int itemCount() const;
    void clearItems();

    [[nodiscard]] std::vector<Light>& lights();
    [[nodiscard]] const std::vector<Light>& lights() const;
    [[nodiscard]] BoundingBox3D sceneBounds() const;

    void addDefaultLights();

private:
    std::vector<std::unique_ptr<PlotItem3D>> items_;
    std::vector<Light> lights_;
};

}  // namespace lumen::plot3d
