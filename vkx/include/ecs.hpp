//
// Created by december on 6/24/22.
//

#pragma once

namespace vkx {
    using Entity = std::uint32_t;

    constexpr static const std::size_t MaxEntity = 256;

    struct Transform {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };

    using ComponentType = std::uint8_t;

    constexpr static const ComponentType MaxComponents = 32;

    using Signature = std::bitset<MaxComponents>;
}
