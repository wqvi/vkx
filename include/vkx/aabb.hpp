#pragma once

namespace vkx {
class AABB {
public:
	AABB() = default;
	
	AABB(const glm::vec3& min, const glm::vec3& max) noexcept;

	bool intersects(const glm::vec3& point) const noexcept;

	bool intersects(const glm::ivec3& point) const noexcept;

private:
	glm::vec3 min = glm::vec3(0, 0, 0);
	glm::vec3 max = glm::vec3(-1, -1, -1);
};
}
