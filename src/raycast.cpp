#include <vkx/raycast.hpp>

template <auto I, auto J>
static constexpr auto derriveStep(float direction) noexcept {
	if (direction > 0) {
		return I;
	} else if (direction < 0) {
		return J;
	}

	return 0;
}

static constexpr glm::ivec3 derriveStep(const glm::vec3& direction) noexcept {
	return {
	    derriveStep<1, -1>(direction.x),
	    derriveStep<1, -1>(direction.y),
	    derriveStep<-1, 1>(direction.z)};
}

static constexpr auto derriveDelta(int step, float direction) noexcept {
	if (step != 0) {
		return 1.0f / glm::abs(direction);
	}

	return std::numeric_limits<float>::infinity();
}

static constexpr glm::vec3 derriveDelta(const glm::ivec3& step, const glm::vec3& direction) noexcept {
	return {
	    derriveDelta(step.x, direction.x),
	    derriveDelta(step.y, direction.y),
	    derriveDelta(step.z, direction.z)};
}

static constexpr auto derriveCross(int step, float origin, float delta) noexcept {
	if (step != 0) {
		if (step == 1) {
			return (glm::ceil(origin) - origin) * delta;
		} else {
			return (origin - glm::floor(origin)) * delta;
		}
	}

	return std::numeric_limits<float>::infinity();
}

static constexpr glm::vec3 derriveCross(const glm::ivec3& step, const glm::vec3& origin, const glm::vec3& delta) noexcept {
	return {
	    derriveCross(step.x, origin.x, delta.x),
	    derriveCross(step.y, origin.y, delta.y),
	    derriveCross(step.z, origin.z, delta.z),
	};
}

static vkx::RaycastResult dda(const glm::vec3& origin, const glm::vec3& direction, float maxLength, vkx::RaycastPredicate predicate) {
	const auto step = derriveStep(direction);
	const auto delta = derriveDelta(step, direction);
	auto cross = derriveCross(step, origin, delta);
	glm::ivec3 hitPos = glm::floor(origin);
	glm::ivec3 previousHitPos = hitPos;
	float length = 0.0f;

	do {
		previousHitPos = hitPos;

		if (cross.x < cross.y) {
			if (cross.x < cross.z) {
				hitPos.x += step.x;

				length = cross.x;

				cross.x += delta.x;
			} else {
				hitPos.z += step.z;

				length = cross.z;

				cross.z += delta.z;
			}
		} else {
			if (cross.y < cross.z) {
				hitPos.y += step.y;

				length = cross.y;

				cross.y += delta.y;
			} else {
				hitPos.z += step.z;

				length = cross.z;

				cross.z += delta.z;
			}
		}

		if (length > maxLength) {
			return vkx::RaycastResult{false, glm::ivec3{0}, glm::ivec3{0}, length};
		}
	} while (!predicate(hitPos));

	return vkx::RaycastResult{true, hitPos, previousHitPos, length};
}

vkx::RaycastResult vkx::raycast(const glm::vec3& origin, const glm::vec3& direction, float maxLength, RaycastPredicate predicate) {
	return dda(origin, direction, maxLength, predicate);
}

vkx::CollisionResult vkx::handleCollision(const AABB& box, const glm::vec3& origin, const glm::vec3& direction, float maxLength, RaycastPredicate predicate) {
	if (box.intersects(origin)) {
		return vkx::CollisionResult{true, origin, glm::vec3{0}};
	}

	float line = 0.0f;
	glm::vec3 hitPos{0};
	glm::vec3 normal{0};

	if (direction.x != 0) {
		if (direction.x > 0) {
			line = (box.min.x - origin.x) / direction.x;
		} else {
			line = (box.max.x - origin.x) / direction.x;
		}

		if (line >= 0 && line <= 1) {
			hitPos = origin + direction * line;
			if ((hitPos.y >= box.min.y) && (hitPos.y <= box.min.y) && (hitPos.z >= box.min.z) && (hitPos.z <= box.min.z)) {
				float d = 1.0f;
				if (direction.x > 0) {
					d = -1.0f;
				}

				normal = glm::vec3{d, 0.0f, 0.0f};
				return vkx::CollisionResult{true, hitPos, normal};
			}
		}
	}

	if (direction.y != 0) {
		if (direction.y > 0) {
			line = (box.min.y - origin.y) / direction.y;
		} else {
			line = (box.max.y - origin.y) / direction.y;
		}

		if (line >= 0 && line <= 1) {
			hitPos = origin + direction * line;
			if ((hitPos.x >= box.min.x) && (hitPos.x <= box.min.x) && (hitPos.z >= box.min.z) && (hitPos.z <= box.min.z)) {
				float d = 1.0f;
				if (direction.y > 0) {
					d = -1.0f;
				}

				normal = glm::vec3{d, 0.0f, 0.0f};
				return vkx::CollisionResult{true, hitPos, normal};
			}
		}
	}

	if (direction.z != 0) {
		if (direction.z > 0) {
			line = (box.min.z - origin.z) / direction.z;
		} else {
			line = (box.max.z - origin.z) / direction.z;
		}

		if (line >= 0 && line <= 1) {
			hitPos = origin + direction * line;
			if ((hitPos.x >= box.min.x) && (hitPos.x <= box.min.x) && (hitPos.y >= box.min.y) && (hitPos.y <= box.min.y)) {
				float d = 1.0f;
				if (direction.z > 0) {
					d = -1.0f;
				}

				normal = glm::vec3{d, 0.0f, 0.0f};
				return vkx::CollisionResult{true, hitPos, normal};
			}
		}
	}

	return vkx::CollisionResult{false, glm::vec3{0}, glm::vec3{0}};
}