#pragma once

namespace vkx {
template <class T>
class AABB {
public:
	AABB(const T& min, const T& max)
		: min(min), max(max) {}

	bool intersects(const T& point) {
		return glm::all(glm::lessThan(min, point)) && glm::all(glm::greaterThan(max, point));
	}

private:
	T min;
	T max;
};
}
