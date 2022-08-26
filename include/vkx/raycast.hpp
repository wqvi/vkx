#pragma once

namespace vkx {
using RaycastPredicate = std::function<bool(const glm::ivec3&)>;

struct RaycastResult {
    bool success = false;
    glm::ivec3 hitPos = glm::ivec3{0};
    glm::ivec3 previousHitPos = glm::ivec3{0};
    float length = 0.0f;
};

RaycastResult raycast(const glm::vec3& origin, const glm::vec3& direction, float maxLength, RaycastPredicate predicate);

bool handleCollision(const glm::vec3& origin, const glm::vec3& direction, float maxLength, RaycastPredicate predicate);
}