#pragma once

namespace vkx {
class AABB {
public:
	AABB() = default;
	
	AABB(const glm::vec3& min, const glm::vec3& max)
		: min(min), max(max) {}

	bool intersects(const glm::vec3& point) {
		return glm::all(glm::lessThan(min, point)) && glm::all(glm::greaterThan(max, point));
	}

private:
	glm::vec3 min = glm::vec3(0, 0, 0);
	glm::vec3 max = glm::vec3(-1, -1, -1);
};
}
