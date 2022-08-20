#pragma once

namespace vkx {
using RaycastPredicate = std::function<bool(const glm::ivec3&)>;

std::tuple<bool, glm::ivec3, glm::ivec3> raycast(const glm::vec3& origin, const glm::vec3& direction, RaycastPredicate predicate);
}