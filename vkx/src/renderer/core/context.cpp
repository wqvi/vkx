#include <renderer/core/context.hpp>
#include <renderer/core/profile.hpp>
#include <renderer/core/queue_config.hpp>
#include <renderer/core/swapchain_info.hpp>
#include <vkx_exceptions.hpp>
#include "debug.hpp"

vkx::RendererContext::RendererContext(std::shared_ptr<vkx::SDLWindow> const &window, const Profile &profile) {
    // Everything that inherits just has one instance of application info as it is the same for everything
    constexpr static const vk::ApplicationInfo applicationInfo{
            "Voxel Game",
            VK_MAKE_VERSION(0, 0, 0),
            "Voxel Engine",
            VK_MAKE_VERSION(0, 0, 0),
            VK_API_VERSION_1_0
    };

    auto instanceExtensions = window->getExtensions();

    vk::InstanceCreateInfo instanceCreateInfo{
            {},
            &applicationInfo,
            profile.layers,
            instanceExtensions
    };

    instance = createInstance(instanceCreateInfo);
}

std::unordered_map<std::uint32_t, vk::PhysicalDevice>
vkx::RendererContext::getPhysicalDevices(const vk::UniqueSurfaceKHR &surface,
                                         const vkx::Profile &profile) const {
    auto physicalDevices = instance->enumeratePhysicalDevices();
    std::unordered_map<std::uint32_t, vk::PhysicalDevice> ratedPhysicalDevices;
    std::ranges::transform(physicalDevices, std::inserter(ratedPhysicalDevices, ratedPhysicalDevices.begin()),
                           [&surface, &profile](const auto &physicalDevice) {
                               return std::make_pair(ratePhysicalDevice(physicalDevice, surface, profile),
                                                     physicalDevice);
                           });
    return ratedPhysicalDevices;
}

vk::PhysicalDevice
vkx::RendererContext::getBestPhysicalDevice(const vk::UniqueSurfaceKHR &surface,
                                            const Profile &profile) const {
    auto physicalDevices = getPhysicalDevices(surface, profile);

    auto location = std::ranges::max_element(physicalDevices,
                                             [](auto const &lhs, auto const &rhs) { return lhs.first < rhs.first; });
    if (location == physicalDevices.end()) {
        throw vkx::VulkanError("Failed to find physical device.");
    }
    return (*location).second;
}

vk::UniqueSurfaceKHR vkx::RendererContext::createSurface(std::shared_ptr<SDLWindow> const &window) const {
    VkSurfaceKHR surface = nullptr;
    if (SDL_Vulkan_CreateSurface(window->cWindow.get(),
                                 *instance,
                                 &surface) != SDL_TRUE) {
        throw vkx::VulkanError("Failure to create VkSurfaceKHR via the SDL2 API.");
    }
    return vk::UniqueSurfaceKHR(surface, *instance);
}

vk::UniqueInstance
vkx::RendererContext::createInstance(const vk::InstanceCreateInfo &instanceCreateInfo) {
#ifdef DEBUG
    auto messageSeverity =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    auto messageType =
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
            {},
            messageSeverity,
            messageType,
            vkDebugCallback,
            nullptr
    };

    vk::StructureChain structureChain{
            instanceCreateInfo,
            debugUtilsMessengerCreateInfo};
    return vk::createInstanceUnique(structureChain.get<vk::InstanceCreateInfo>());
#else
    return vk::createInstanceUnique(instanceCreateInfo);
#endif
}

std::uint32_t
vkx::RendererContext::ratePhysicalDevice(const vk::PhysicalDevice &physicalDevice,
                                         const vk::UniqueSurfaceKHR &surface,
                                         const Profile &profile) {
    std::uint32_t rating = 0;
    if (profile.validateExtensions(physicalDevice)) {
        rating++;
    }

    if (QueueConfig indices{physicalDevice, surface}; indices.isComplete()) {
        rating++;
    }

    if (SwapchainInfo info{physicalDevice, surface}; info.isComplete()) {
        rating++;
    }

    if (physicalDevice.getFeatures().samplerAnisotropy) {
        rating++;
    }

    return rating;
}
