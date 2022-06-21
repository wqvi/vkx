#pragma once

namespace vkx
{
  struct Profile
  {
    Profile(std::vector<char const *> const &requestedLayers, std::vector<char const *> const &requestedExtensions);

    std::vector<char const *> layers;
    std::vector<char const *> extensions;

    [[nodiscard]] bool validateExts(vk::PhysicalDevice const &physicalDevice) const;

    static bool validateLayers(std::vector<char const *> const &layers);

    static Profile createDefault();

  private:
    static bool isSubset(std::vector<std::string> const &array, std::vector<char const *> const &subset);

    static bool compareStrings(char const *lhs, char const *rhs);
  };
}