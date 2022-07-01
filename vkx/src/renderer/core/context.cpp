#include <renderer/core/context.hpp>
#include <renderer/core/profile.hpp>
#include <renderer/core/queue_config.hpp>
#include <renderer/core/swapchain_info.hpp>
#include "debug.hpp"
#include <SDL2/SDL_vulkan.h>

vkx::RendererContext::RendererContext(SDL_Window *window) {
    const vkx::Profile profile = vkx::Profile{};

    constexpr static const vk::ApplicationInfo applicationInfo{
            "Voxel Game",             // pApplicationName
            VK_MAKE_VERSION(1, 0, 0), // applicationVersion
            "vkx",                    // engineVersion
            VK_MAKE_VERSION(1, 0, 0), // engineVersion
            VK_API_VERSION_1_0        // apiVersion
    };

    auto instanceExtensions = getWindowExtensions(window);

    const vk::InstanceCreateInfo instanceCreateInfo{
            {},                // flags
            &applicationInfo,  // applicationInfo
            profile.layers,    // pEnabledLayerNames
            instanceExtensions // pEnabledExtensionNames
    };

    instance = createInstance(instanceCreateInfo);
}

vkx::RendererContext::RendererContext(const vkx::SDLWindow &window, const Profile &profile) {
    // Everything that inherits just has one instance of application info as it is the same for everything
    constexpr static const vk::ApplicationInfo applicationInfo{
            "Voxel Game",             // pApplicationName
            VK_MAKE_VERSION(0, 0, 0), // applicationVersion
            "Voxel Engine",           // engineVersion
            VK_MAKE_VERSION(0, 0, 0), // engineVersion
            VK_API_VERSION_1_0        // apiVersion
    };

    auto instanceExtensions = window.getExtensions();

    const vk::InstanceCreateInfo instanceCreateInfo{
            {},                // flags
            &applicationInfo,  // applicationInfo
            profile.layers,    // pEnabledLayerNames
            instanceExtensions // pEnabledExtensionNames
    };

    instance = createInstance(instanceCreateInfo);
}

namespace vkx {
    RendererContext::RendererContext(SDL_Window *window, const Profile &profile) {
        constexpr static const vk::ApplicationInfo applicationInfo{
                "Voxel Game",             // pApplicationName
                VK_MAKE_VERSION(1, 0, 0), // applicationVersion
                "Voxel Engine",           // engineVersion
                VK_MAKE_VERSION(1, 0, 0), // engineVersion
                VK_API_VERSION_1_0        // apiVersion
        };

        auto instanceExtensions = getWindowExtensions(window);

        const vk::InstanceCreateInfo instanceCreateInfo{
                {},                // flags
                &applicationInfo,  // applicationInfo
                profile.layers,    // pEnabledLayerNames
                instanceExtensions // pEnabledExtensionNames
        };

        instance = createInstance(instanceCreateInfo);
    }

    std::unordered_map<std::uint32_t, vk::PhysicalDevice>
    RendererContext::getPhysicalDevices(const vk::UniqueSurfaceKHR &surface, const vkx::Profile &profile) const {
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
    RendererContext::getBestPhysicalDevice(const vk::UniqueSurfaceKHR &surface, const Profile &profile) const {
        auto physicalDevices = getPhysicalDevices(surface, profile);

        auto location = std::max_element(physicalDevices.begin(), physicalDevices.end(), [](auto const &lhs, auto const &rhs)
        { return lhs.first < rhs.first; });
        if (location == physicalDevices.end())
        {
            throw std::runtime_error("Failed to find physical device.");
        }
        return (*location).second;
    }

    std::vector<const char *> RendererContext::getWindowExtensions(SDL_Window *window) {
        std::uint32_t sdlExtensionCount = 0;

        if (SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr) != SDL_TRUE) {
            throw std::runtime_error("Failure to get SDL extension count.");
        }

        std::vector<const char *> instanceExtensions(sdlExtensionCount);

        if (SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, instanceExtensions.data()) != SDL_TRUE) {
            throw std::runtime_error("Failure to enumerate SDL extensions.");
        }

#ifdef DEBUG
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
        return instanceExtensions;
    }

    vk::UniqueInstance RendererContext::createInstance(const vk::InstanceCreateInfo &instanceCreateInfo) {
#ifdef DEBUG
        auto messageSeverity =
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        auto messageType =
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
                {},              // flags
                messageSeverity, // messageSeverity
                messageType,     // messageType
                vkDebugCallback, // pfnUserCallback
                nullptr          // pUserData
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
    RendererContext::ratePhysicalDevice(const vk::PhysicalDevice &physicalDevice, const vk::UniqueSurfaceKHR &surface,
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
}