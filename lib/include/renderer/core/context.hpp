#pragma once

#include <renderer/core/renderer_types.hpp>
#include <renderer/core/profile.hpp>

namespace vkx
{
    class RendererContext
    {
    public:
        RendererContext(Profile const &profile);

        [[nodiscard]] std::unordered_map<std::uint32_t, vk::PhysicalDevice> getPhysicalDevices(vk::UniqueSurfaceKHR const &surface) const;

    protected:
        vk::UniqueInstance instance;
        
    private:
        Profile profile;

        static std::vector<char const*> getWindowExtensions();

        static vk::UniqueInstance createInstance(vk::InstanceCreateInfo const &instanceCreateInfo);

        static std::uint32_t ratePhysicalDevice(vk::PhysicalDevice const &physicalDevice, vk::UniqueSurfaceKHR const &surface, Profile const &profile);
    };
}