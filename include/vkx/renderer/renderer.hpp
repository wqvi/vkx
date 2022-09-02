#pragma once

#include <vkx/renderer/core/bootstrap.hpp>
#include <vkx/renderer/core/device.hpp>
#include <vkx/application.hpp>

namespace vkx {
class Renderer {
private:
    RendererBootstrap bootstrap{};
    Device device{};
    Allocator allocator{};
    CommandSubmitter commandSubmitter{};
    Swapchain swapchain{};
    vk::UniqueRenderPass clearRenderPass{};
    vk::UniqueRenderPass loadRenderPass{};

    std::vector<vkx::GraphicsPipeline> pipelines{};

public:
    Renderer() = default;

    explicit Renderer(const SDLWindow& window);

    void attachPipeline(const GraphicsPipelineInformationTest& pipelineInformation);
};
} // namespace vkx