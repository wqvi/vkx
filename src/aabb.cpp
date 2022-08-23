#include <vkx/aabb.hpp>

vkx::AABB::AABB(const glm::vec3& min, const glm::vec3& max) noexcept
    : min(min), max(max) {}

bool vkx::AABB::intersects(const glm::vec3 &point) const noexcept {
    return glm::all(glm::lessThan(min, point)) && glm::all(glm::greaterThan(max, point));
}

bool vkx::AABB::intersects(const glm::ivec3 &point) const noexcept {
    glm::vec3 tmp = point;
    return glm::all(glm::lessThan(min, tmp)) && glm::all(glm::greaterThan(max, tmp));
}