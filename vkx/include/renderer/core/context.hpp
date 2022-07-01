#pragma once

#include <renderer/core/renderer_types.hpp>
#include <renderer/core/profile.hpp>
#include <window.hpp>

namespace vkx {
    class RendererContext {
    public:
        RendererContext();

        explicit RendererContext(SDL_Window *window);

        explicit RendererContext(SDL_Window *window, const Profile &profile);

        explicit RendererContext(const SDLWindow &window, const Profile &profile);

        [[nodiscard]]
        std::unordered_map<std::uint32_t, vk::PhysicalDevice>
        getPhysicalDevices(const vk::UniqueSurfaceKHR &surface,
                           const vkx::Profile &profile) const;

        [[nodiscard]]
        vk::PhysicalDevice
        getBestPhysicalDevice(const vk::UniqueSurfaceKHR &surface, const vkx::Profile &profile) const;

    protected:
        vk::UniqueInstance instance;

        static std::vector<const char *> getWindowExtensions(SDL_Window *window);

        static vk::UniqueInstance createInstance(const vk::InstanceCreateInfo &instanceCreateInfo);

        static std::uint32_t ratePhysicalDevice(const vk::PhysicalDevice &physicalDevice,
                                                const vk::UniqueSurfaceKHR &surface,
                                                const Profile &profile);
    };
}