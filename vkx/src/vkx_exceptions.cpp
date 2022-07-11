//
// Created by december on 6/30/22.
//

#include <vkx/vkx_exceptions.hpp>

const char *vkx::SDLError::what() const noexcept {
    return SDL_GetError();
}

vkx::VulkanError::VulkanError(const char *message)
        : message(message) {}

vkx::VulkanError::VulkanError(vk::Result result)
        : message(vk::to_string(result)) {}

const char *vkx::VulkanError::what() const noexcept {
    return message.c_str();
}