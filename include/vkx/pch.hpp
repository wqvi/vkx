#pragma once

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_vulkan.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <functional>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef NDEBUG
#define RELEASE
#else
#define DEBUG
#endif

namespace vkx {
static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = UINT32_C(2);

static constexpr VkDescriptorPoolSize UNIFORM_BUFFER_POOL_SIZE{
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    MAX_FRAMES_IN_FLIGHT};

static constexpr VkDescriptorPoolSize SAMPLER_BUFFER_POOL_SIZE{
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    MAX_FRAMES_IN_FLIGHT};
} // namespace vkx
