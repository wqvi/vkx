#pragma once

#include <vkx/aabb.hpp>

namespace vkx {
using RaycastPredicate = std::function<bool(const glm::ivec3&)>;

struct RaycastResult {
    bool success = false;
    glm::ivec3 hitPos = glm::ivec3{0};
    glm::ivec3 previousHitPos = glm::ivec3{0};
    float length = 0.0f;
};

struct CollisionResult {
    bool success = false;
    glm::vec3 hitPos = glm::vec3{0};
    glm::vec3 normal = glm::vec3{0};
};

RaycastResult raycast(const glm::vec3& origin, const glm::vec3& direction, float maxLength, RaycastPredicate predicate);

CollisionResult handleCollision(const AABB& box, const glm::vec3& origin, const glm::vec3& direction, float maxLength, RaycastPredicate predicate);
}