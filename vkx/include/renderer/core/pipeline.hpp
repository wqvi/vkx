#pragma once

#include "device.hpp"

namespace vkx
{
    class PipelineShader {
    public:
        explicit PipelineShader(const Device &device, const std::string &file);

    private:
        static std::vector<std::uint32_t> readFile(const std::string &filename);

        vk::UniqueDescriptorSetLayout descriptorSetLayout;
    };

  class Pipeline
  {
  public:
    Pipeline() = default;

    explicit Pipeline(Device const &device, vk::UniqueDescriptorSetLayout const &descriptorSetLayout);

    vk::UniquePipelineLayout layout;
    vk::UniquePipeline pipeline;
  
  protected:
    static std::vector<char> readFile(std::string const &filename);

    static vk::UniqueShaderModule createShaderModule(vk::UniqueDevice const &device, std::vector<char> const &code);
  };

  class ComputePipeline : public Pipeline
  {
  public:
    ComputePipeline() = default;

    ComputePipeline(Device const &device, vk::UniqueDescriptorSetLayout const &descriptorSetLayout);
  };

  class GraphicsPipeline : public Pipeline
  {
  public:
    GraphicsPipeline() = default;

    GraphicsPipeline(Device const &device, vk::Extent2D const &extent, vk::UniqueRenderPass const &renderPass, vk::UniqueDescriptorSetLayout const &descriptorSetLayout);
  };
}
