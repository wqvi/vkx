#include "vkx/renderer/core/pipeline.hpp"
#include "vkx/renderer/core/vertex.hpp"
#include <vkx/vkx.hpp>

auto createShaderDescriptorSetLayout(vk::Device device) {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding(
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr);

	constexpr vk::DescriptorSetLayoutBinding samplerLayoutBinding(
	    1,
	    vk::DescriptorType::eCombinedImageSampler,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr);

	constexpr vk::DescriptorSetLayoutBinding lightLayoutBinding(
	    2,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr);

	constexpr vk::DescriptorSetLayoutBinding materialLayoutBinding(
	    3,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr);

	constexpr std::array bindings{uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding, materialLayoutBinding};

	const vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings);
	return device.createDescriptorSetLayoutUnique(layoutInfo);
};

auto createHighlightDescriptorSetLayout(vk::Device device) {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding(
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr);

	constexpr std::array bindings{uboLayoutBinding};

	const vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings);
	return device.createDescriptorSetLayoutUnique(layoutInfo);
}

auto getBindingDescription() noexcept {
	std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};

	bindingDescriptions.push_back({0, sizeof(glm::vec3), vk::VertexInputRate::eVertex});

	return bindingDescriptions;
}

auto getAttributeDescriptions() noexcept {
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};

	attributeDescriptions.push_back({0, 0, vk::Format::eR32G32B32Sfloat, 0});

	return attributeDescriptions;
}

int main(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failure to initialize SDL2: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	int width = 640;
	int height = 480;

	const auto windowFlags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;

	SDL_Window* const window = SDL_CreateWindow("Jewelry", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, windowFlags);

	if (window == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failure to initialize SDL2 window: %s", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	if (SDL_SetRelativeMouseMode(SDL_TRUE)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failure to capture mouse: %s", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	{
		vkx::Camera camera({0, 0, 0});

		const vkx::RendererBootstrap renderer(window);
		const auto device = renderer.createDevice();
		const auto allocator = device->createAllocator();
		auto swapchain = device->createSwapchain(window, allocator);
		const auto clearRenderPass = device->createRenderPass(swapchain->imageFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR, vk::AttachmentLoadOp::eClear);
		// const auto loadRenderPass = device->createRenderPass(swapchain->imageFormat, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::AttachmentLoadOp::eLoad);

		swapchain->createFramebuffers(static_cast<vk::Device>(*device), *clearRenderPass);

		constexpr vk::DescriptorPoolSize uniformBufferDescriptor(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT);
		constexpr vk::DescriptorPoolSize samplerBufferDescriptor(vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT);

		constexpr std::array poolSizes{uniformBufferDescriptor, samplerBufferDescriptor, uniformBufferDescriptor, uniformBufferDescriptor};

		const vk::DescriptorPoolCreateInfo poolInfo({}, MAX_FRAMES_IN_FLIGHT, poolSizes);

		const auto descriptorPool = (*device)->createDescriptorPoolUnique(poolInfo);

		const auto descriptorSetLayout = createShaderDescriptorSetLayout(static_cast<vk::Device>(*device));

		const auto highlightDescriptorSetLayout = createHighlightDescriptorSetLayout(static_cast<vk::Device>(*device));

		vkx::GraphicsPipelineInformation graphicsPipelineInformation = {
		    "shader.vert.spv",
		    "shader.frag.spv",
		    swapchain->extent,
		    *clearRenderPass,
		    *descriptorSetLayout,
		    vkx::Vertex::getBindingDescription(),
		    vkx::Vertex::getAttributeDescriptions()};

		auto graphicsPipeline = device->createGraphicsPipeline(graphicsPipelineInformation);

		vkx::GraphicsPipelineInformation highlightGraphicsPipelineInformation = {
		    "highlight.vert.spv",
		    "highlight.frag.spv",
		    swapchain->extent,
		    *clearRenderPass,
		    *descriptorSetLayout,
			getBindingDescription(),
			getAttributeDescriptions()
		};

		auto highlightGraphicsPipeline = device->createGraphicsPipeline(highlightGraphicsPipelineInformation);
		const auto commandSubmitter = device->createCommandSubmitter();
		constexpr std::uint32_t drawCommandAmount = 1;
		constexpr std::uint32_t secondaryDrawCommandAmount = 4;
		const auto drawCommands = commandSubmitter->allocateDrawCommands(drawCommandAmount);
		const auto secondaryDrawCommands = commandSubmitter->allocateSecondaryDrawCommands(secondaryDrawCommandAmount);
		const auto syncObjects = vkx::SyncObjects::createSyncObjects(static_cast<vk::Device>(*device));

		vkx::VoxelChunk<16> chunk({0, 0, 0});
		for (int j = 0; j < 10; j++) {
			for (int k = 0; k < 4; k++) {
				for (int i = 0; i < 14; i++) {
					chunk.voxels.set(j, k, i, vkx::Voxel::Air);
				}
			}
		}

		chunk.greedy();

		vkx::VoxelChunk<16> chunk1({1, 0, 0});
		chunk1.greedy();

		vkx::VoxelChunk<16> chunk2({0, 0, 1});
		chunk2.greedy();

		vkx::VoxelChunk<16> chunk3({1, 0, 1});
		chunk3.greedy();

		vkx::Mesh mesh(chunk.ve, chunk.in, allocator);
		mesh.indexCount = std::distance(chunk.in.begin(), chunk.indexIter);

		vkx::Mesh mesh1(chunk1.ve, chunk1.in, allocator);
		mesh1.indexCount = std::distance(chunk1.in.begin(), chunk1.indexIter);

		vkx::Mesh mesh2(chunk2.ve, chunk2.in, allocator);
		mesh2.indexCount = std::distance(chunk2.in.begin(), chunk2.indexIter);

		vkx::Mesh mesh3(chunk3.ve, chunk3.in, allocator);
		mesh3.indexCount = std::distance(chunk3.in.begin(), chunk3.indexIter);

		const vkx::Texture texture("a.jpg", *device, allocator, commandSubmitter);

		auto mvpBuffers = allocator->allocateUniformBuffers(vkx::MVP{});
		auto lightBuffers = allocator->allocateUniformBuffers(vkx::DirectionalLight{});
		auto materialBuffers = allocator->allocateUniformBuffers(vkx::Material{});

		const std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
		const vk::DescriptorSetAllocateInfo allocInfo(*descriptorPool, layouts);

		const auto descriptorSets = (*device)->allocateDescriptorSets(allocInfo);

		for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			const std::array descriptorWrites{
			    mvpBuffers[i].createWriteDescriptorSet(descriptorSets[i], 0),
			    texture.createWriteDescriptorSet(descriptorSets[i], 1),
			    lightBuffers[i].createWriteDescriptorSet(descriptorSets[i], 2),
			    materialBuffers[i].createWriteDescriptorSet(descriptorSets[i], 3),
			};

			(*device)->updateDescriptorSets(descriptorWrites, {});
		}

		auto proj = glm::perspective(70.0f, 640.0f / 480.0f, 0.1f, 100.0f);

		std::uint32_t currentFrame = 0;
		SDL_Event event{};
		bool isRunning = true;
		bool framebufferResized = false;
		SDL_ShowWindow(window);
		while (isRunning) {
			camera.position += camera.direction * 0.001f;

			auto& mvpBuffer = mvpBuffers[currentFrame];
			mvpBuffer->model = glm::mat4(1.0f);
			mvpBuffer->view = camera.viewMatrix();
			mvpBuffer->proj = proj;

			auto& lightBuffer = lightBuffers[currentFrame];
			lightBuffer->position = glm::vec3(1.0f, 3.0f, 1.0f);
			lightBuffer->eyePosition = camera.position;
			lightBuffer->ambientColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.2f);
			lightBuffer->diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
			lightBuffer->specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
			lightBuffer->constant = 1.0f;
			lightBuffer->linear = 0.09f;
			lightBuffer->quadratic = 0.032f;

			auto& materialBuffer = materialBuffers[currentFrame];
			materialBuffer->specularColor = glm::vec3(0.2f);
			materialBuffer->shininess = 100.0f;

			const auto& syncObject = syncObjects[currentFrame];
			syncObject.waitForFence();
			auto [result, imageIndex] = swapchain->acquireNextImage(static_cast<vk::Device>(*device), syncObject);

			if (result == vk::Result::eErrorOutOfDateKHR) {
				int newWidth;
				int newHeight;
				SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
				while (newWidth == 0 || newHeight == 0) {
					SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
					SDL_WaitEvent(nullptr);
				}

				(*device)->waitIdle();

				swapchain = device->createSwapchain(window, allocator);

				swapchain->createFramebuffers(static_cast<vk::Device>(*device), *clearRenderPass);

				graphicsPipelineInformation = {
				    "shader.vert.spv",
				    "shader.frag.spv",
				    swapchain->extent,
				    *clearRenderPass,
				    *descriptorSetLayout,
				    vkx::Vertex::getBindingDescription(),
				    vkx::Vertex::getAttributeDescriptions()};

				graphicsPipeline = device->createGraphicsPipeline(graphicsPipelineInformation);
				continue;
			} else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
				throw std::runtime_error("Failed to acquire next image.");
			}

			mvpBuffer.mapMemory();
			lightBuffer.mapMemory();
			materialBuffer.mapMemory();

			syncObject.resetFence();

			const vkx::DrawInfo drawInfo = {
			    *clearRenderPass,
			    *swapchain->framebuffers[imageIndex],
			    swapchain->extent,
			    *graphicsPipeline->pipeline,
			    *graphicsPipeline->layout,
			    descriptorSets[currentFrame],
			    {mesh.vertex->object, mesh1.vertex->object, mesh2.vertex->object, mesh3.vertex->object},
			    {mesh.index->object, mesh1.index->object, mesh2.index->object, mesh3.index->object},
			    {static_cast<std::uint32_t>(mesh.indexCount), static_cast<std::uint32_t>(mesh1.indexCount), static_cast<std::uint32_t>(mesh2.indexCount), static_cast<std::uint32_t>(mesh3.indexCount)}};

			const vk::CommandBuffer* begin = &drawCommands[currentFrame * drawCommandAmount];

			const vk::CommandBuffer* secondaryBegin = &secondaryDrawCommands[currentFrame * secondaryDrawCommandAmount];

			commandSubmitter->recordDrawCommands(begin, drawCommandAmount, secondaryBegin, secondaryDrawCommandAmount, drawInfo);

			commandSubmitter->submitDrawCommands(begin, drawCommandAmount, syncObject);

			commandSubmitter->presentToSwapchain(*swapchain->swapchain, imageIndex, syncObject);

			if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
				framebufferResized = false;

				int newWidth;
				int newHeight;
				SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
				while (newWidth == 0 || newHeight == 0) {
					SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
					SDL_WaitEvent(nullptr);
				}

				(*device)->waitIdle();

				swapchain = device->createSwapchain(window, allocator);

				swapchain->createFramebuffers(static_cast<vk::Device>(*device), *clearRenderPass);

				graphicsPipelineInformation = {
				    "shader.vert.spv",
				    "shader.frag.spv",
				    swapchain->extent,
				    *clearRenderPass,
				    *descriptorSetLayout,
				    vkx::Vertex::getBindingDescription(),
				    vkx::Vertex::getAttributeDescriptions()};

				graphicsPipeline = device->createGraphicsPipeline(graphicsPipelineInformation);
			} else if (result != vk::Result::eSuccess) {
				throw std::runtime_error("Failed to present.");
			}

			currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

			bool valid = false;
			glm::ivec3 location(0, 0, 0);
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_QUIT:
					isRunning = false;
					break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
						framebufferResized = true;
						width = event.window.data1;
						height = event.window.data2;
						proj = glm::perspective(70.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
					}
					break;
				case SDL_MOUSEMOTION:
					camera.updateMouse({-event.motion.xrel, -event.motion.yrel});
					break;
				case SDL_KEYDOWN:
					camera.updateKey(event.key.keysym.sym);
					break;
				case SDL_KEYUP:
					camera.direction = glm::vec3(0);
					break;
				case SDL_MOUSEBUTTONDOWN:
					std::tie(valid, location) = chunk.raycast(camera, width, height);
					if (valid) {
						chunk.voxels.set(location.x, location.y, location.z, vkx::Voxel::Air);
						chunk.greedy();
						mesh.vertex->mapMemory(chunk.ve);
						mesh.index->mapMemory(chunk.in);
						mesh.indexCount = std::distance(chunk.in.begin(), chunk.indexIter);
					}
					// chunk1.raycast(camera, width, height);
					// chunk2.raycast(camera, width, height);
					// chunk3.raycast(camera, width, height);
					break;
				default:
					break;
				}
			}
		}

		(*device)->waitIdle();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
