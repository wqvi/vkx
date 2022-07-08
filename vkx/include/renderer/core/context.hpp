#pragma once

#include <renderer/core/renderer_types.hpp>

namespace vkx {
    class RendererContext {
    public:
        RendererContext() = default;

        explicit RendererContext(std::shared_ptr<SDLWindow> const &window,
                                 Profile const &profile);

        [[nodiscard]]
        std::unordered_map<std::uint32_t, vk::PhysicalDevice>
        getPhysicalDevices(vk::UniqueSurfaceKHR const &surface,
                           vkx::Profile const &profile) const;

        [[nodiscard]]
        vk::PhysicalDevice
        getBestPhysicalDevice(vk::UniqueSurfaceKHR const &surface,
                              vkx::Profile const &profile) const;

        [[nodiscard("Do not discard an integral Vulkan component")]]
        vk::UniqueSurfaceKHR
        createSurface(std::shared_ptr<SDLWindow> const &window) const;

    private:
        vk::UniqueInstance instance;

        static vk::UniqueInstance
        createInstance(vk::InstanceCreateInfo const &instanceCreateInfo);

        static std::uint32_t
        ratePhysicalDevice(vk::PhysicalDevice const &physicalDevice,
                           vk::UniqueSurfaceKHR const &surface,
                           Profile const &profile);
    };
}