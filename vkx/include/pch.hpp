#pragma once

#define VULKAN_HPP_NO_SPACESHIP_OPERATOR
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_log.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <array>
#include <vector>
#include <set>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <optional>
#include <unordered_map>
#include <chrono>

#ifdef NDEBUG
#define RELEASE
#else
#define DEBUG
#endif