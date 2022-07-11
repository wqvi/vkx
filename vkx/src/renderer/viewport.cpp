//
// Created by december on 7/5/22.
//

#include <vkx/renderer/viewport.hpp>

vkx::Viewport::operator const glm::mat4 &() const {
    return projection;
}

std::uint32_t vkx::Viewport::getFOV() const noexcept {
    return fov;
}

void vkx::Viewport::setFOV(std::uint32_t _fov) noexcept {
    fov = _fov;
    projection = glm::perspective(
            glm::radians(static_cast<glm::f32>(fov)),
            static_cast<glm::f32>(width) / static_cast<glm::f32>(height),
            0.1f,
            100.0f);
    projection[1][1] *= -1.0f;
}

void vkx::Viewport::setSize(Sint32 _width, Sint32 _height) {
    width = _width;
    height = _height;
    projection = glm::perspective(
            glm::radians(static_cast<glm::f32>(fov)),
            static_cast<glm::f32>(width) / static_cast<glm::f32>(height),
            0.1f,
            100.0f);
    projection[1][1] *= -1.0f;
}
