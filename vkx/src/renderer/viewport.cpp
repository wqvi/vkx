//
// Created by december on 7/5/22.
//

#include <renderer/viewport.hpp>

vkx::Viewport::Viewport(std::uint32_t fov, Sint32 width, Sint32 height)
        : fov(fov),
          projection(
                  glm::perspective(
                          glm::radians(static_cast<glm::f32>(fov)),
                          static_cast<glm::f32>(width) / static_cast<glm::f32>(height),
                          0.1f,
                          100.0f)),
          width(width),
          height(height) {
    projection[1][1] *= -1.0f;
}

vkx::Viewport::operator const glm::mat4 &() const {
    return projection;
}

std::uint32_t vkx::Viewport::getFOV() const noexcept {
    return fov;
}

void vkx::Viewport::setFOV(std::uint32_t newFOV) noexcept {
    fov = newFOV;
    projection = glm::perspective(
            glm::radians(static_cast<glm::f32>(fov)),
            static_cast<glm::f32>(width) / static_cast<glm::f32>(height),
            0.1f,
            100.0f);
    projection[1][1] *= -1.0f;
}

void vkx::Viewport::setSize(Sint32 newWidth, Sint32 newHeight) {
    width = newWidth;
    height = newHeight;
    projection = glm::perspective(
            glm::radians(static_cast<glm::f32>(fov)),
            static_cast<glm::f32>(width) / static_cast<glm::f32>(height),
            0.1f,
            100.0f);
    projection[1][1] *= -1.0f;
}
