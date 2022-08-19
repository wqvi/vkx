#include "vkx/voxels/voxel_matrix.hpp"
#include "vkx/voxels/voxel_types.hpp"
#include <glm/common.hpp>
#include <glm/fwd.hpp>
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

static auto dda(const glm::vec3& origin, const glm::vec3& direction, const vkx::VoxelMatrix& voxels) {
	constexpr float rayLength = 4.0f;

	glm::ivec3 hitPos = glm::floor(origin);

	const auto step = derriveStep(direction);
	const auto delta = derriveDelta(step, direction);
	auto cross = derriveCross(step, origin, delta);

	do {
		if (cross.x < cross.y) {
			if (cross.x < cross.z) {
				hitPos.x += step.x;

				if (cross.x > rayLength) {
					return std::make_pair(false, glm::ivec3(0));
				}

				cross.x += delta.x;
			} else {
				hitPos.z += step.z;

				if (cross.z > rayLength) {
					return std::make_pair(false, glm::ivec3(0));
				}

				cross.z += delta.z;
			}
		} else {
			if (cross.y < cross.z) {
				hitPos.y += step.y;

				if (cross.y > rayLength) {
					return std::make_pair(false, glm::ivec3(0));
				}

				cross.y += delta.y;
			} else {
				hitPos.z += step.z;

				if (cross.z > rayLength) {
					return std::make_pair(false, glm::ivec3(0));
				}

				cross.z += delta.z;
			}
		}

	} while (voxels.at(hitPos) == vkx::Voxel::Air);

	return std::make_pair(true, hitPos);
}

std::pair<bool, glm::ivec3> vkx::raycast(const glm::vec3& chunkPosition, const Camera& camera, const VoxelMatrix& voxels) {
	const auto origin = glm::vec3(chunkPosition) - camera.position;

	const auto orientation = camera.yawOrientation * camera.pitchOrientation;
	const auto front = orientation * glm::quat(0.0f, 0.0f, 0.0f, -1.0f) * glm::conjugate(orientation);

	const glm::vec3 direction{front.x, front.y, -front.z};

	return dda(origin, direction, voxels);
}