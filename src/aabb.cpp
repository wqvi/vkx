#include <vkx/aabb.hpp>

vkx::AABB::AABB(const glm::vec3& min, const glm::vec3& max)
    : min(min), max(max) {}

void vkx::AABB::move(const glm::vec3 &pos) {
    min += pos;
    max += pos;
}

bool vkx::AABB::intersects(const glm::vec3& point) const {
	return glm::all(glm::lessThan(min, point)) && glm::all(glm::greaterThan(max, point));
}

bool vkx::AABB::intersects(const glm::ivec3& point) const {
	glm::vec3 tmp = point;
	return glm::all(glm::lessThan(min, tmp)) && glm::all(glm::greaterThan(max, tmp));
}

bool vkx::AABB::intersects(const AABB& other) const {
	return other.intersects(min) || other.intersects(max);
}
