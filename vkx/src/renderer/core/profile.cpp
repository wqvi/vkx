#include <renderer/core/profile.hpp>

namespace vkx {
    [[maybe_unused]] Profile::Profile(const std::vector<char const *> &requestedLayers,
                                      const std::vector<char const *> &requestedExtensions) {
        if (!requestedLayers.empty()) {
            layers.insert(layers.end(), requestedLayers.begin(), requestedLayers.end());

            std::sort(layers.begin(), layers.end(), compareStrings);
            layers.erase(std::unique(layers.begin(), layers.end(), compareStrings), layers.end());
        }

        if (!validateLayers(layers)) {
            throw std::invalid_argument("Failure to provide valid layers.");
        }

        if (!requestedExtensions.empty()) {
            extensions.insert(extensions.end(), requestedExtensions.begin(), requestedExtensions.end());

            std::sort(extensions.begin(), extensions.end(), compareStrings);
            extensions.erase(std::unique(extensions.begin(), extensions.end(), compareStrings), extensions.end());
        }
    }

    bool Profile::validateExtensions(const vk::PhysicalDevice &physicalDevice) const {
        auto extensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
        std::vector<std::string_view> stringExtensions;
        std::transform(extensionProperties.begin(), extensionProperties.end(), std::back_inserter(stringExtensions),
                       [](auto const &props) { return props.extensionName; });
        return isSubset(stringExtensions, extensions);
    }

    bool Profile::validateLayers(const std::vector<char const *> &layers) {
        auto currentLayers = vk::enumerateInstanceLayerProperties();
        std::vector<std::string_view> currentStrLayers;
        std::transform(currentLayers.begin(), currentLayers.end(), std::back_inserter(currentStrLayers),
                       [](auto const &props) { return props.layerName; });
        return isSubset(currentStrLayers, layers);
    }

    bool Profile::isSubset(const std::vector<std::string_view> &arr, const std::vector<char const *> &subset) {
        std::size_t i = 0;
        for (auto const &subsetStr: subset) {
            for (i = 0; i < arr.size(); i++) {
                if (arr[i] == subsetStr) {
                    break;
                }
            }

            if (i == arr.size()) {
                return false;
            }
        }
        return true;
    }

    bool Profile::compareStrings(const char *lhs, const char *rhs) {
        return std::strcmp(lhs, rhs) < 0;
    }
}