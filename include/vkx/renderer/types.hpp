#pragma once

namespace vkx {
class Buffer;
class CommandSubmitter;
struct DrawInfo;
namespace pipeline {
class VulkanPipeline;
class GraphicsPipeline;
struct GraphicsPipelineInformation;
class ComputePipeline;
struct ComputePipelineInformation;
} // namespace pipeline
class Image;
struct QueueConfig;
class Swapchain;
struct SwapchainInfo;
struct SyncObjects;
class Texture;
class UniformBuffer;
struct Vertex;
class VulkanAllocationDeleter;
class VulkanAllocator;
struct VulkanAllocatorDeleter;
class VulkanDevice;
class VulkanInstance;
class VulkanPoolDeleter;
class VulkanRenderPass;
} // namespace vkx