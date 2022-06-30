//
// Created by december on 6/30/22.
//

#pragma once

#include <vkx_types.hpp>
#include <renderer/core/renderer_types.hpp>

namespace vkx {
    class SDLError : public std::exception {
    public:
        [[nodiscard]] const char *what() const noexcept override;
    };

    class VulkanError : public std::exception {
    public:
        explicit VulkanError(const char *message);

        explicit VulkanError(vk::Result result);

        [[nodiscard]] const char *what() const noexcept override;

    private:
        std::string message;
    };
}