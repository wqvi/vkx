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

	return INFINITY;
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

	return INFINITY;
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

vkx::RaycastResult vkx::raycastAABB(const glm::vec3 &origin, const glm::vec3 &direction, float maxLength, RaycastPredicate predicate) {
	
}