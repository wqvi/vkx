#include <renderer/core/profile.hpp>

namespace vkx
{
  Profile::Profile(std::vector<char const *> const &requestedLayers, std::vector<char const *> const &requestedExtensions)
      : layers({
#ifdef DEBUG
            "VK_LAYER_KHRONOS_validation"
#endif
        }),
        extensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME})
  {
    if (requestedLayers.size() > 0)
    {
      layers.insert(layers.end(), requestedLayers.begin(), requestedLayers.end());

      std::sort(layers.begin(), layers.end(), compareStrings);
      layers.erase(std::unique(layers.begin(), layers.end(), compareStrings), layers.end());
    }

    if (!validateLayers(layers))
    {
      throw std::invalid_argument("Failure to provide valid layers.");
    }

    if (requestedExtensions.size() > 0)
    {
      extensions.insert(extensions.end(), requestedExtensions.begin(), requestedExtensions.end());

      std::sort(extensions.begin(), extensions.end(), compareStrings);
      extensions.erase(std::unique(extensions.begin(), extensions.end(), compareStrings), extensions.end());
    }
  }

  bool Profile::validateExts(vk::PhysicalDevice const &physicalDevice) const
  {
    auto currentExts = physicalDevice.enumerateDeviceExtensionProperties();
    std::vector<std::string> currentStrExts;
    std::transform(currentExts.begin(), currentExts.end(), std::back_inserter(currentStrExts), [](auto const &props)
                   { return props.extensionName; });
    return isSubset(currentStrExts, extensions);
  }

  bool Profile::validateLayers(std::vector<char const *> const &layers)
  {
    auto currentLayers = vk::enumerateInstanceLayerProperties();
    std::vector<std::string> currentStrLayers;
    std::transform(currentLayers.begin(), currentLayers.end(), std::back_inserter(currentStrLayers), [](auto const &props)
                   { return props.layerName; });
    return isSubset(currentStrLayers, layers);
  }

  Profile Profile::createDefault()
  {
    return {{}, {}};
  }

  bool Profile::isSubset(std::vector<std::string> const &arr, std::vector<char const *> const &subset)
  {
    std::size_t i = 0;
    for (auto const &subsetStr : subset)
    {
      for (i = 0; i < arr.size(); i++)
      {
        if (arr[i] == subsetStr)
        {
          break;
        }
      }

      if (i == arr.size())
      {
        return false;
      }
    }
    return true;
  }

  bool Profile::compareStrings(char const *lhs, char const *rhs)
  {
    return std::strcmp(lhs, rhs) < 0;
  }
}