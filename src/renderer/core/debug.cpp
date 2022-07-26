#ifdef DEBUG

#include <vkx/debug.hpp>
#include <iostream>
#include <vulkan/vulkan.h>

extern "C" {
VkBool32 vkDebugCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                         [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
                         [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                         [[maybe_unused]] void *pUserData) {
    std::cerr << "[ Vulkan Error ]\n";
    std::cerr << pCallbackData->pMessage << "\n";
    std::cerr << "This happened in the [" << pUserData << "] object\n";
    return VK_FALSE;
}

VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator,
                                        VkDebugUtilsMessengerEXT *pCallback) {
    // Compliant cast to the create function
    auto func = std::bit_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                     VkDebugUtilsMessengerEXT callback,
                                     const VkAllocationCallbacks *pAllocator) {
    // Compliant cast to the destroy function
    auto func = std::bit_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        func(instance, callback, pAllocator);
    }
}
}
#endif