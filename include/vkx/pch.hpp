#pragma once

#define VULKAN_HPP_NO_SPACESHIP_OPERATOR
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_vulkan.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <forward_list>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <list>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#ifdef NDEBUG
#define RELEASE
#else
#define DEBUG
#endif

namespace vkx {
static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = UINT32_C(2);

static constexpr vk::DescriptorPoolSize UNIFORM_BUFFER_POOL_SIZE{vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT};
static constexpr vk::DescriptorPoolSize SAMPLER_BUFFER_POOL_SIZE{vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT};

static constexpr std::array<glm::vec3, 24> CUBE_VERTICES = {
    glm::vec3{0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},

    {1.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 1.0f},

    {1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 1.0f},
    {0.0f, 0.0f, 1.0f},

    {0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f},

    {0.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 1.0f},

    {0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 0.0f}};

static constexpr std::array<std::uint32_t, 36> CUBE_INDICES = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20};
} // namespace vkx