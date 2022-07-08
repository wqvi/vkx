#pragma once

#include <renderer/core/renderer_types.hpp>
#include <renderer/core/profile.hpp>

namespace vkx {
    class RendererContext {
    public:
        RendererContext() = default;

        explicit RendererContext(std::shared_ptr<SDLWindow> const &window, const Profile &profile);

        [[nodiscard]]
        std::unordered_map<std::uint32_t, vk::PhysicalDevice>
        getPhysicalDevices(const vk::UniqueSurfaceKHR &surface,
                           const vkx::Profile &profile) const;

        [[nodiscard]]
        vk::PhysicalDevice
        getBestPhysicalDevice(const vk::UniqueSurfaceKHR &surface,
                              const vkx::Profile &profile) const;

        [[nodiscard("Do not discard an integral Vulkan component")]]
        vk::UniqueSurfaceKHR
        createSurface(std::shared_ptr<SDLWindow> const &window) const;

    private:
        vk::UniqueInstance instance;

        static vk::UniqueInstance
        createInstance(const vk::InstanceCreateInfo &instanceCreateInfo);

        static std::uint32_t
        ratePhysicalDevice(const vk::PhysicalDevice &physicalDevice,
                                                const vk::UniqueSurfaceKHR &surface,
                                                const Profile &profile);
    };
}