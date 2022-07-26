#include <vkx/dual_contour/density.hpp>

#include <glm/gtc/noise.hpp>

struct NoiseData
{
	std::int32_t octaves;
	float frequency;
	float lacunarity;
	float persistence;
};

auto calculateSphere(glm::vec3 const &worldPosition, glm::vec3 const &origin, float radius)
{
	return glm::length(worldPosition - origin) - radius;
}

auto calculateCuboid(glm::vec3 const &worldPosition, glm::vec3 const &origin, glm::vec3 const &halfDimensions)
{
	auto const &local_pos = worldPosition - origin;
	auto const &pos = local_pos;

	auto const &d = glm::abs(pos) - halfDimensions;
	float const m = glm::max(d.x, glm::max(d.y, d.z));
	return glm::min(m, glm::length(glm::max(d, glm::vec3(0.f))));
}

auto calculateFractalNoise(NoiseData const &noiseData, glm::vec2 const &position)
{
	float const SCALE = 1.0f / 128.0f;
	auto p = position * SCALE;
	float noise = 0.0f;

	float amplitude = 1.0f;
	p *= noiseData.frequency;

	for (std::int32_t i = 0; i < noiseData.octaves; i++)
	{
		noise += glm::simplex(p) * amplitude;
		p *= noiseData.lacunarity;
		amplitude *= noiseData.persistence;
	}

	// move into [0, 1] range
	return 0.5f + (0.5f * noise);
}

float densityFunction(const glm::vec3 &worldPosition)
{
	float const MAX_HEIGHT = 20.0f;
	
	NoiseData const noiseData{
		4,		 // octaves
		0.5343f, // frequency
		2.2324f, // lacunarity
		0.68324f // persistence
	};

	auto const noise = calculateFractalNoise(noiseData, glm::vec2(worldPosition.x, worldPosition.z));
	float const terrain = worldPosition.y - (MAX_HEIGHT * noise);

	auto const cube = calculateCuboid(worldPosition, glm::vec3(-4.0, 10.0f, -4.0f), glm::vec3(12.0f));
	auto const sphere = calculateSphere(worldPosition, glm::vec3(15.0f, 2.5f, 1.0f), 16.0f);

	return glm::max(-cube, glm::min(sphere, terrain));
}
