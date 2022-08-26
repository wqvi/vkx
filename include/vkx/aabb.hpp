#pragma once

namespace vkx {
struct AABB {
	glm::vec3 min = glm::vec3{0, 0, 0};
	glm::vec3 max = glm::vec3{-1, -1, -1};

	AABB() = default;

	AABB(const glm::vec3& min, const glm::vec3& max);

	void move(const glm::vec3& pos);

	bool intersects(const glm::vec3& point) const;

	bool intersects(const glm::ivec3& point) const;

	bool intersects(const AABB& other) const;
};
} // namespace vkx
