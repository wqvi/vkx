#pragma once

#ifdef DEBUG
extern "C" {
VkBool32
vkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData, void *pUserData);
}
#endif