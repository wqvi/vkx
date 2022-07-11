#include <vkx/renderer/core/commands.hpp>

#include <vkx/renderer/buffer.hpp>
#include <vkx/renderer/core/device.hpp>

namespace vkx
{
  SingleTimeCommand::SingleTimeCommand(Device const &device, vk::Queue const &queue)
      : queue(queue),
        device(static_cast<vk::Device>(device)),
        commandPool(static_cast<vk::CommandPool>(device))
  {
    vk::CommandBufferAllocateInfo allocInfo{
        commandPool,                      // commandPool
        vk::CommandBufferLevel::ePrimary, // level
        1                                 // commandBufferCount
    };

    commandBuffer = device->allocateCommandBuffers(allocInfo)[0];

    vk::CommandBufferBeginInfo beginInfo{
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit // flags
    };

    commandBuffer.begin(beginInfo);
  }

  SingleTimeCommand::~SingleTimeCommand()
  {
    commandBuffer.end();

    vk::SubmitInfo submitInfo{
        {},            // waitSemaphores
        {},            // waitDstStageMask
        commandBuffer, // commandBuffers
        {}             // signalSemaphores
    };

    static_cast<void>(queue.submit(1, &submitInfo, {}));
    queue.waitIdle();

    device.freeCommandBuffers(commandPool, commandBuffer);
  }

  SingleTimeCommand::operator vk::CommandBuffer const &() const
  {
    return commandBuffer;
  }

  vk::CommandBuffer const *SingleTimeCommand::operator->() const
  {
    return &commandBuffer;
  }

  DrawCommand::DrawCommand(vk::UniqueDevice const &device, vk::CommandBuffer const &commandBuffer)
      : device(*device), commandBuffer(commandBuffer) {}

  DrawCommand::DrawCommand(Device const &device, vk::CommandBuffer const &commandBuffer)
      : device(static_cast<vk::Device>(device)), commandBuffer(commandBuffer) {}

  DrawCommand::operator vk::CommandBuffer const &() const
  {
    return commandBuffer;
  }

  vk::CommandBuffer const *DrawCommand::operator->() const
  {
    return &commandBuffer;
  }

  void DrawCommand::record(vk::RenderPass const &renderPass,
                           vk::Framebuffer const &framebuffer,
                           vk::Extent2D const &extent,
                           vk::Pipeline const &graphicsPipeline,
                           vk::PipelineLayout const &graphicsPipelineLayout,
                           vk::DescriptorSet const &descriptorSet,
                           VertexBuffer const &vertexBuffer,
                           IndexBuffer const &indexBuffer,
                           std::uint32_t indicesCount) const
  {
    commandBuffer.reset({});
    vk::CommandBufferBeginInfo beginInfo{};
    static_cast<void>(commandBuffer.begin(beginInfo));

    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].color = std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    vk::Rect2D renderArea{
        {
            0, // x
            0  // y
        },     // offset
        extent // extent
    };

    vk::RenderPassBeginInfo renderPassInfo{
        renderPass,  // renderPass
        framebuffer, // framebuffer
        renderArea,  // renderArea
        clearValues  // clearValues
    };

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    commandBuffer.bindVertexBuffers(0, static_cast<vk::Buffer>(vertexBuffer), {0});

    commandBuffer.bindIndexBuffer(static_cast<vk::Buffer>(indexBuffer), 0, vk::IndexType::eUint32);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipelineLayout, 0, descriptorSet, {});

    commandBuffer.drawIndexed(indicesCount, 1, 0, 0, 0);

    commandBuffer.endRenderPass();

    commandBuffer.end();
  }
}
