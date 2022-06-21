#pragma once

#include <renderer/core/renderer_types.hpp>
#include <renderer/core/profile.hpp>

namespace vkx {
    class RendererContext {
    public:
        explicit RendererContext(const Profile &profile);

        [[nodiscard]] std::unordered_map<std::uint32_t, vk::PhysicalDevice>
        getPhysicalDevices(const vk::UniqueSurfaceKHR &surface) const;

    protected:
        vk::UniqueInstance instance;

    private:
        const Profile profile;

        static std::vector<const char *> getWindowExtensions();

        static vk::UniqueInstance createInstance(const vk::InstanceCreateInfo &instanceCreateInfo);

        static std::uint32_t
        ratePhysicalDevice(const vk::PhysicalDevice &physicalDevice, const vk::UniqueSurfaceKHR &surface,
                           const Profile &profile);
    };
}