#include <renderer/core/context.hpp>
#include <renderer/core/profile.hpp>
#include <renderer/core/queue_config.hpp>
#include <renderer/core/swapchain_info.hpp>
#include "debug.hpp"

namespace vkx {
    RendererContext::RendererContext(const Profile &profile)
            : profile(profile) {
        constexpr static const vk::ApplicationInfo applicationInfo{
                "Voxel Game",             // pApplicationName
                VK_MAKE_VERSION(1, 0, 0), // applicationVersion
                "Voxel Engine",           // engineVersion
                VK_MAKE_VERSION(1, 0, 0), // engineVersion
                VK_API_VERSION_1_0        // apiVersion
        };

        auto instanceExtensions = getWindowExtensions();

        const vk::InstanceCreateInfo instanceCreateInfo{
                {},                // flags
                &applicationInfo,  // applicationInfo
                profile.layers,    // pEnabledLayerNames
                instanceExtensions // pEnabledExtensionNames
        };

        instance = createInstance(instanceCreateInfo);
    }

    std::unordered_map<std::uint32_t, vk::PhysicalDevice>
    RendererContext::getPhysicalDevices(const vk::UniqueSurfaceKHR &surface) const {
        auto physicalDevices = instance->enumeratePhysicalDevices();
        std::unordered_map<std::uint32_t, vk::PhysicalDevice> ratedPhysicalDevices;
        std::ranges::transform(physicalDevices, std::inserter(ratedPhysicalDevices, ratedPhysicalDevices.begin()),
                               [&surface, &profile = this->profile](const auto &physicalDevice) {
                                   return std::make_pair(ratePhysicalDevice(physicalDevice, surface, profile),
                                                         physicalDevice);
                               });
        return ratedPhysicalDevices;
    }

    std::vector<char const *> RendererContext::getWindowExtensions() {
        std::uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char *> instanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

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
        if (profile.validateExts(physicalDevice)) {
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