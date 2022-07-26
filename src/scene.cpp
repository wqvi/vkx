//
// Created by december on 7/5/22.
//

#include <vkx/scene.hpp>

vkx::Viewport &vkx::Scene::getViewport() {
    return viewport;
}

vkx::Camera &vkx::Scene::getCamera() {
    return camera;
}
