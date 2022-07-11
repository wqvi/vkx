#include <vkx/renderer/core/queue_config.hpp>

#include <vkx/renderer/core/device.hpp>

namespace vkx
{
    QueueConfig::QueueConfig(vk::PhysicalDevice const &physicalDevice, vk::UniqueSurfaceKHR const &surface)
    {
        auto queueFamilies = physicalDevice.getQueueFamilyProperties();

        for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(queueFamilies.size()); i++)
        {
            auto flags = queueFamilies[i].queueFlags;
            if (flags & vk::QueueFlagBits::eCompute)
            {
                computeIndex = i;
            }

            if (flags & vk::QueueFlagBits::eGraphics)
            {
                graphicsIndex = i;
            }

            if (physicalDevice.getSurfaceSupportKHR(i, *surface))
            {
                presentIndex = i;
            }

            if (isComplete())
            {
                break;
            }
        }

        if (isUniversal() && isComplete())
        {
            std::set<std::uint32_t> uniqueIndices{
                graphicsIndex.value(),
                computeIndex.value(),
                presentIndex.value()};

            std::ranges::copy(uniqueIndices.begin(), uniqueIndices.end(), std::back_inserter(indices));
        }
    }

    QueueConfig::QueueConfig(Device const &device, vk::UniqueSurfaceKHR const &surface)
        : QueueConfig(static_cast<vk::PhysicalDevice>(device), surface) {}

    bool QueueConfig::isComplete() const
    {
        return graphicsIndex.has_value() && computeIndex.has_value() && presentIndex.has_value();
    }

    bool QueueConfig::isUniversal() const
    {
        return *computeIndex == *graphicsIndex && *graphicsIndex == *presentIndex;
    }

    std::vector<vk::DeviceQueueCreateInfo> QueueConfig::createQueueInfos(vk::ArrayProxy<float> const &queuePriorities) const
    {
        if (queuePriorities.size() <= 0)
        {
            throw std::invalid_argument("The queuePriorities parameter's size must not be zero or less.");
        }

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        for (std::uint32_t index : indices)
        {
            vk::DeviceQueueCreateInfo queueCreateInfo{
                {},                     // flags
                index,                  // queueFamilyIndex
                queuePriorities.size(), // queueCount
                queuePriorities.data()  // pQueuePriorities
            };
            queueCreateInfos.push_back(queueCreateInfo);
        }

        return queueCreateInfos;
    }

    vk::SharingMode QueueConfig::getImageSharingMode() const
    {
        if (isUniversal())
        {
            return vk::SharingMode::eExclusive;
        }
        return vk::SharingMode::eConcurrent;
    }

    Queues::Queues(Device const &device, QueueConfig const &queueConfig)
        : compute(device->getQueue(queueConfig.computeIndex.value(), 0)),
        graphics(device->getQueue(queueConfig.graphicsIndex.value(), 0)),
        present(device->getQueue(queueConfig.presentIndex.value(), 0)) {}
}