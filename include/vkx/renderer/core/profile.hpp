#pragma once

namespace vkx {
    struct Profile {
        Profile() = default;

        [[maybe_unused]]
        Profile(const std::vector<char const *> &requestedLayers, const std::vector<char const *> &requestedExtensions);

        [[nodiscard]]
        bool validateExtensions(const vk::PhysicalDevice &physicalDevice) const;

        static bool validateLayers(const std::vector<char const *> &layers);

        std::vector<char const *> layers = {
#ifdef DEBUG
                // Necessary only for debugging
                "VK_LAYER_KHRONOS_validation"
#endif
        };

        std::vector<char const *> extensions = {
                // Necessary for displaying graphics to the framebuffer
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

    private:
        static bool isSubset(const std::vector<std::string_view> &array, const std::vector<const char *> &subset);

        // Reusable function for std::algorithms simply compares two strings in a specific manner
        static bool compareStrings(const char *lhs, const char *rhs);
    };
}