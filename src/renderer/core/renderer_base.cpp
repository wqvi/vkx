#include <vkx/renderer/core/renderer_base.hpp>

#include <iostream>
#include <vkx/debug.hpp>
#include <vkx/renderer/core/queue_config.hpp>
#include <vkx/renderer/core/swapchain_info.hpp>
#include <vkx/renderer/model.hpp>
#include <vkx/renderer/uniform_buffer.hpp>
#include <vkx/vkx_exceptions.hpp>
#include <vulkan/vulkan_core.h>

static bool isSubset(const std::vector<const char*>& arr,
		     const std::vector<const char*>& subset) {
	auto iter = arr.begin();
	const auto end = arr.end();

	if (arr.size() < subset.size()) {
		throw std::invalid_argument("Arr must be larger than subset.");
	}

	for (const char* subsetStr : subset) {
		for (iter = arr.begin(); iter != end; iter++) {
			if (std::strcmp(*iter, subsetStr) == 0)
				break;
		}

		if (iter == end) {
			return false;
		}
	}

	return true;
}

vkx::RendererBase::RendererBase(SDL_Window* window) : window(window) {
	static constexpr vk::ApplicationInfo applicationInfo{
	    "Jewelry", VK_MAKE_VERSION(0, 0, 1), "Vulcan", VK_MAKE_VERSION(0, 0, 1),
	    VK_API_VERSION_1_0};

	std::uint32_t count = 0;
	if (SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr) != SDL_TRUE) {
		throw vkx::SDLError();
	}
	std::vector<const char*> extensions(count);
	if (SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data()) !=
	    SDL_TRUE) {
		throw vkx::SDLError();
	}

#ifdef DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	std::vector<const char*> layers{
#ifdef DEBUG
	    "VK_LAYER_KHRONOS_validation",
#endif
	};

#ifdef DEBUG
	auto availableLayers = vk::enumerateInstanceLayerProperties();
	std::vector<const char*> currentStrLayers;
	std::transform(availableLayers.begin(), availableLayers.end(),
		       std::back_inserter(currentStrLayers),
		       [](const auto& props) { return props.layerName; });
	if (!isSubset(currentStrLayers, layers)) {
		throw vkx::VulkanError("Failed to find requested Vulkan instance layers.");
	}
#endif

	vk::InstanceCreateInfo instanceCreateInfo{
	    {}, &applicationInfo, layers, extensions};

#ifdef DEBUG
	auto messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
			       vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			       vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	auto messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			   vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			   vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
	    {}, messageSeverity, messageType, vkDebugCallback, nullptr};

	vk::StructureChain structureChain{instanceCreateInfo,
					  debugUtilsMessengerCreateInfo};

	instance =
	    vk::createInstanceUnique(structureChain.get<vk::InstanceCreateInfo>());
#else
	instance = vk::createInstanceUnique(instanceCreateInfo);
#endif

	VkSurfaceKHR cSurface = nullptr;
	if (SDL_Vulkan_CreateSurface(window, *instance, &cSurface) != SDL_TRUE) {
		throw vkx::SDLError();
	}
	surface = vk::UniqueSurfaceKHR(cSurface, *instance);

	auto physicalDevices = instance->enumeratePhysicalDevices();
	vk::PhysicalDevice bestPhysicalDevice = nullptr;
	std::uint32_t bestRating = 0;
	for (const auto& pDevice : physicalDevices) {
		std::uint32_t rating = 0;

		auto extensionProperties = pDevice.enumerateDeviceExtensionProperties();
		std::vector<const char*> stringExtensions;
		std::transform(extensionProperties.begin(), extensionProperties.end(),
			       std::back_inserter(stringExtensions),
			       [](const auto& props) { return props.extensionName; });
		if (isSubset(stringExtensions, extensions)) {
			rating++;
		}

		if (QueueConfig indices{pDevice, surface}; indices.isComplete()) {
			rating++;
		}

		if (SwapchainInfo info{pDevice, surface}; info.isComplete()) {
			rating++;
		}

		if (pDevice.getFeatures().samplerAnisotropy) {
			rating++;
		}

		if (rating > bestRating) {
			bestRating = rating;
			bestPhysicalDevice = pDevice;
		}
	}

	if (!static_cast<bool>(bestPhysicalDevice)) {
		throw vkx::VulkanError("Failure to initialize device.");
	}

	device = std::make_unique<vkx::Device>(instance, bestPhysicalDevice, surface);

	createSwapchain();

	vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0, vk::DescriptorType::eUniformBuffer, 1,
	    vk::ShaderStageFlagBits::eVertex, nullptr};

	vk::DescriptorSetLayoutBinding samplerLayoutBinding{
	    1, vk::DescriptorType::eCombinedImageSampler, 1,
	    vk::ShaderStageFlagBits::eFragment, nullptr};

	vk::DescriptorSetLayoutBinding lightLayoutBinding{
	    2, vk::DescriptorType::eUniformBuffer, 1,
	    vk::ShaderStageFlagBits::eFragment, nullptr};

	vk::DescriptorSetLayoutBinding materialLayoutBinding{
	    3, vk::DescriptorType::eUniformBuffer, 1,
	    vk::ShaderStageFlagBits::eFragment, nullptr};

	std::vector<vk::DescriptorSetLayoutBinding> bindings{
	    uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding,
	    materialLayoutBinding};

	vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, bindings};

	descriptorSetLayout = (*device)->createDescriptorSetLayoutUnique(layoutInfo);

	graphicsPipeline = GraphicsPipeline{*device, swapchain.extent, renderPass,
					    descriptorSetLayout};

	drawCommands = device->createDrawCommands(MAX_FRAMES_IN_FLIGHT);

	syncObjects = SyncObjects::createSyncObjects(*device);

	createDescriptorPool();
}

namespace vkx {
void RendererBase::recreateSwapchain() {
	int width, height;
	SDL_Vulkan_GetDrawableSize(window, &width, &height);
	while (width == 0 || height == 0) {
		SDL_Vulkan_GetDrawableSize(window, &width, &height);
		SDL_WaitEvent(nullptr);
	}

	(*device)->waitIdle();

	createSwapchain();
	graphicsPipeline = GraphicsPipeline{*device, swapchain.extent, renderPass,
					    descriptorSetLayout};
}

void RendererBase::createDescriptorPool() {
	vk::DescriptorPoolSize uniformBufferDescriptor{
	    vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT};

	std::array<vk::DescriptorPoolSize, 4> poolSizes{};
	std::fill(poolSizes.begin(), poolSizes.end(), uniformBufferDescriptor);

	poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;

	vk::DescriptorPoolCreateInfo poolInfo{{}, MAX_FRAMES_IN_FLIGHT, poolSizes};

	descriptorPool = (*device)->createDescriptorPoolUnique(poolInfo);
}

void RendererBase::createDescriptorSets(
    const std::vector<UniformBuffer<MVP>>& mvpBuffers,
    const std::vector<UniformBuffer<DirectionalLight>>& lightBuffers,
    const std::vector<UniformBuffer<Material>>& materialBuffers,
    const Texture& texture) {
	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
						     *descriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo{*descriptorPool, layouts};

	descriptorSets = (*device)->allocateDescriptorSets(allocInfo);

	for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		std::array<vk::WriteDescriptorSet, 4> descriptorWrites{
		    mvpBuffers[i].createWriteDescriptorSet(descriptorSets[i], 0),
		    texture.createWriteDescriptorSet(descriptorSets[i], 1),
		    lightBuffers[i].createWriteDescriptorSet(descriptorSets[i], 2),
		    materialBuffers[i].createWriteDescriptorSet(descriptorSets[i], 3),
		};

		(*device)->updateDescriptorSets(descriptorWrites, {});
	}
}

void RendererBase::drawFrame(const UniformBuffer<MVP>& mvpBuffer,
			     const UniformBuffer<DirectionalLight>& lightBuffer,
			     const UniformBuffer<Material>& materialBuffer,
			     const VertexBuffer& vertexBuffer,
			     const IndexBuffer& indexBuffer,
			     std::uint32_t indexCount,
			     std::uint32_t& currentIndexFrame) {
	static_cast<void>((*device)->waitForFences(
	    *syncObjects[currentIndexFrame].inFlightFence, true, UINT64_MAX));
	auto [result, imageIndex] = swapchain.acquireNextImage(
	    *device, syncObjects[currentIndexFrame].imageAvailableSemaphore);

	if (result == vk::Result::eErrorOutOfDateKHR) {
		recreateSwapchain();
		return;
	} else if (result != vk::Result::eSuccess &&
		   result != vk::Result::eSuboptimalKHR) {
		throw vkx::VulkanError(result);
	}

	mvpBuffer.mapMemory();
	lightBuffer.mapMemory();
	materialBuffer.mapMemory();

	(*device)->resetFences(*syncObjects[currentIndexFrame].inFlightFence);

	drawCommands[currentIndexFrame].record(
	    *renderPass, *swapchain.framebuffers[imageIndex], swapchain.extent,
	    *graphicsPipeline.pipeline, *graphicsPipeline.layout,
	    descriptorSets[currentIndexFrame], vertexBuffer, indexBuffer, indexCount);

	std::vector<vk::CommandBuffer> commandBuffers{
	    static_cast<vk::CommandBuffer>(drawCommands[currentIndexFrame])};
	device->submit(commandBuffers,
		       *syncObjects[currentIndexFrame].imageAvailableSemaphore,
		       *syncObjects[currentIndexFrame].renderFinishedSemaphore,
		       *syncObjects[currentIndexFrame].inFlightFence);

	result =
	    device->present(swapchain, imageIndex,
			    *syncObjects[currentIndexFrame].renderFinishedSemaphore);

	if (result == vk::Result::eErrorOutOfDateKHR ||
	    result == vk::Result::eSuboptimalKHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapchain();
	} else if (result != vk::Result::eSuccess) {
		throw vkx::VulkanError(result);
	}

	currentIndexFrame = (currentIndexFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

std::uint32_t RendererBase::getCurrentFrameIndex() const {
	return currentFrame;
}

void RendererBase::createSwapchain() {
	swapchain = vkx::Swapchain{*device, surface, window, swapchain};

	renderPass = createRenderPass();

	swapchain.createFramebuffers(*device, renderPass);
}

vk::UniqueRenderPass
RendererBase::createRenderPass(vk::AttachmentLoadOp loadOp) const {
	vk::AttachmentDescription colorAttachment{
	    {},				      // flags
	    swapchain.imageFormat,	      // format
	    vk::SampleCountFlagBits::e1,      // samples
	    loadOp,			      // loadOp
	    vk::AttachmentStoreOp::eStore,    // storeOp
	    vk::AttachmentLoadOp::eDontCare,  // stencilLoadOp
	    vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
	    vk::ImageLayout::eUndefined,      // initialLayout
	    vk::ImageLayout::ePresentSrcKHR   // finalLayout
	};

	vk::AttachmentReference colorAttachmentRef{
	    0,					     // attachment
	    vk::ImageLayout::eColorAttachmentOptimal // layout
	};

	vk::AttachmentDescription depthAttachment{
	    {},						    // flags
	    device->findDepthFormat(),			    // format
	    vk::SampleCountFlagBits::e1,		    // samples
	    vk::AttachmentLoadOp::eClear,		    // loadOp
	    vk::AttachmentStoreOp::eDontCare,		    // storeOp
	    vk::AttachmentLoadOp::eDontCare,		    // stencilLoadOp
	    vk::AttachmentStoreOp::eDontCare,		    // stencilStoreOp
	    vk::ImageLayout::eUndefined,		    // initialLayout
	    vk::ImageLayout::eDepthStencilAttachmentOptimal // finalLayout
	};

	vk::AttachmentReference depthAttachmentRef{
	    1,						    // attachment
	    vk::ImageLayout::eDepthStencilAttachmentOptimal // layout
	};

	vk::SubpassDescription subpass{
	    {},				      // flags
	    vk::PipelineBindPoint::eGraphics, // pipelineBindPoint
	    {},				      // inputAttachments
	    colorAttachmentRef,		      // colorAttachments
	    {},				      // resolveAttachments
	    &depthAttachmentRef,	      // pDepthStencilAttachment
	    {}				      // preserveAttachments
	};

	auto dependencyStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
				   vk::PipelineStageFlagBits::eEarlyFragmentTests;
	auto dependencyAccessMask = vk::AccessFlagBits::eColorAttachmentWrite |
				    vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	vk::SubpassDependency dependency{
	    VK_SUBPASS_EXTERNAL, // srcSubpass
	    0,			 // dstSubpass
	    dependencyStageMask, // srcStageMask
	    dependencyStageMask, // dstStageMask
	    {},			 // srcAccessMask
	    dependencyAccessMask // dstAccessMask
	};

	std::vector renderPassAttachments{colorAttachment, depthAttachment};

	vk::RenderPassCreateInfo renderPassInfo{
	    {},			   // flags
	    renderPassAttachments, // attachments
	    subpass,		   // subpasses
	    dependency		   // dependencies
	};

	return (*device)->createRenderPassUnique(renderPassInfo);
}
} // namespace vkx

vkx::Mesh vkx::RendererBase::allocateMesh(
    const std::vector<Vertex>& vertices,
    const std::vector<std::uint32_t>& indices) const {
	return vkx::Mesh{vertices, indices, *device};
}

vkx::Texture
vkx::RendererBase::allocateTexture(const std::string& textureFile) const {
	return vkx::Texture{textureFile, *device};
}

void vkx::RendererBase::waitIdle() const { (*device)->waitIdle(); }
