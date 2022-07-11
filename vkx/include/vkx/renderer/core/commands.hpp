#pragma once

#include "renderer_types.hpp"

namespace vkx
{
  class SingleTimeCommand
  {
  public:
    SingleTimeCommand(Device const &device, vk::Queue const &queue);

    ~SingleTimeCommand();

    explicit operator vk::CommandBuffer const &() const;

    vk::CommandBuffer const *operator->() const;


  private:
    vk::Queue queue;
    vk::Device device;
    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;
  };

  class DrawCommand
  {
  public:
    DrawCommand() = default;

    DrawCommand(vk::UniqueDevice const &device,
                vk::CommandBuffer const &commandBuffer);

    DrawCommand(Device const &device, vk::CommandBuffer const &commandBuffer);

    explicit operator vk::CommandBuffer const &() const;

    vk::CommandBuffer const *operator->() const;

    void record(vk::RenderPass const &renderPass,
                vk::Framebuffer const &framebuffer,
                vk::Extent2D const &extent,
                vk::Pipeline const &graphicsPipeline,
                vk::PipelineLayout const &graphicsPipelineLayout,
                vk::DescriptorSet const &descriptorSet,
                VertexBuffer const &vertexBuffer,
                IndexBuffer const &indexBuffer,
                std::uint32_t indicesCount) const;

  private:
    vk::Queue queue;
    vk::Device device;
    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;
  };
}