#pragma once

#include <vkx/renderer/model.hpp>
#include <vkx/renderer/core/bootstrap.hpp>
#include <vkx/renderer/core/device.hpp>
#include <vkx/application.hpp>

namespace vkx {
class Renderer {
private:
    RendererBootstrap bootstrap{};
    Device device{};
public:
    Allocator allocator{};
    CommandSubmitter commandSubmitter{};
private:
    Swapchain swapchain{};
    vk::UniqueRenderPass clearRenderPass{};
    vk::UniqueRenderPass loadRenderPass{};

    std::vector<vkx::GraphicsPipeline> pipelines{};

public:
    std::vector<vkx::SyncObjects> syncObjects{};

public:
    Renderer() = default;

    explicit Renderer(const SDLWindow& window);

    vkx::GraphicsPipeline* attachPipeline(const GraphicsPipelineInformationTest& pipelineInformation);

    void resized(const SDLWindow& window);

    template <class T, class K>
    auto createMesh(const T& vertices, const K& indices) const {
        return vkx::Mesh{vertices, indices, allocator};
    }

    vkx::Texture createTexture(const std::string& file) const;
};
} // namespace vkx