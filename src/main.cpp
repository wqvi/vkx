#include <vkx/vkx.hpp>

auto createShaderDescriptorSetLayout(const vkx::Device& device) {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr};

	constexpr vk::DescriptorSetLayoutBinding samplerLayoutBinding{
	    1,
	    vk::DescriptorType::eCombinedImageSampler,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr};

	constexpr vk::DescriptorSetLayoutBinding lightLayoutBinding{
	    2,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr};

	constexpr vk::DescriptorSetLayoutBinding materialLayoutBinding{
	    3,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eFragment,
	    nullptr};

	constexpr std::array bindings = {uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding, materialLayoutBinding};

	const vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, bindings};
	return device->createDescriptorSetLayoutUnique(layoutInfo);
};

auto createHighlightDescriptorSetLayout(const vkx::Device& device) {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr};

	constexpr std::array bindings = {uboLayoutBinding};

	const vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, bindings};
	return device->createDescriptorSetLayoutUnique(layoutInfo);
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

struct MyScene : public vkx::Scene {
	vkx::Camera camera{};
	vkx::VoxelChunk<16> chunk;
	vkx::Mesh mesh;
	glm::mat4 highlightModel{1.0f};
	vkx::Texture texture;

	vkx::RaycastPredicate a;

	void init(const vkx::Device& device, const std::shared_ptr<vkx::Allocator>& allocator, const std::shared_ptr<vkx::CommandSubmitter>& commandSubmitter) override {
		a = [&chunk = this->chunk](const auto& b) {
			return chunk.at(b) != vkx::Voxel::Air;
		};

		texture = vkx::Texture{"a.jpg", device, allocator, commandSubmitter};
	}

	void destroy() override {
		// Nothing
	}

	void update() override {
		// Do something
		camera.position += camera.direction * 0.001f;
	}

	void windowResize(int width, int height) override {
		
	}

	void mouseMoved(const SDL_MouseMotionEvent& event) override {
		camera.updateMouse({-event.xrel, -event.yrel});
		const auto origin = glm::vec3(chunk.normalizedPosition) - camera.position;

		const auto raycastResult = vkx::raycast(origin, camera.front, 4.0f, a);
		if (raycastResult.success) {
			highlightModel = glm::translate(glm::mat4(1.0f), glm::vec3(chunk.normalizedPosition - raycastResult.hitPos) + glm::vec3(-1.0f));
		}
	}

	void mousePressed(const SDL_MouseButtonEvent& event) override {
		const auto origin = glm::vec3(chunk.normalizedPosition) - camera.position;
		const auto raycastResult = vkx::raycast(origin, camera.front, 4.0f, a);
		if (raycastResult.success) {
			chunk.set(raycastResult.hitPos, vkx::Voxel::Air);
			chunk.greedy();
			mesh.vertex->mapMemory(chunk.vertices);
			mesh.index->mapMemory(chunk.indices);
			mesh.indexCount = std::distance(chunk.indices.begin(), chunk.indexIter);
		}
	}

	void mouseReleased(const SDL_MouseButtonEvent& event) override {
	}

	void keyPressed(const SDL_KeyboardEvent& event) override {
		camera.updateKey(event.keysym.sym);
		const auto origin = glm::vec3(chunk.normalizedPosition) - camera.position;
		const vkx::AABB box{glm::vec3{0}, glm::vec3{1}};
		const auto collision = vkx::handleCollision(box, origin, glm::vec3{0, 1, 0}, 1.0f, a);
		if (collision.success) {
			SDL_Log("Colliding");
		}
	}

	void keyReleased(const SDL_KeyboardEvent& event) override {
		camera.direction = glm::vec3(0);
	}
};

int main(void) {
	// vkx::Application application{};
	// application.setScene(new MyScene{});
	// application.run();

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
		const auto allocator = device.createAllocator();
		auto swapchain = device.createSwapchain(window, allocator);
		const auto clearRenderPass = device.createRenderPass(swapchain->imageFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear);
		const auto loadRenderPass = device.createRenderPass(swapchain->imageFormat, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::AttachmentLoadOp::eLoad);

		swapchain->createFramebuffers(device, *clearRenderPass);

		constexpr vk::DescriptorPoolSize uniformBufferDescriptor{vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT};
		constexpr vk::DescriptorPoolSize samplerBufferDescriptor{vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT};

		constexpr std::array poolSizes = {uniformBufferDescriptor, samplerBufferDescriptor, uniformBufferDescriptor, uniformBufferDescriptor};

		const vk::DescriptorPoolCreateInfo poolInfo{{}, MAX_FRAMES_IN_FLIGHT, poolSizes};

		const auto descriptorPool = device->createDescriptorPoolUnique(poolInfo);

		constexpr std::array highlightPoolSizes = {uniformBufferDescriptor};

		const vk::DescriptorPoolCreateInfo highlightPoolInfo{{}, MAX_FRAMES_IN_FLIGHT, highlightPoolSizes};

		const auto highlightDescriptorPool = device->createDescriptorPoolUnique(poolInfo);

		const auto descriptorSetLayout = createShaderDescriptorSetLayout(device);

		const auto highlightDescriptorSetLayout = createHighlightDescriptorSetLayout(device);

		vkx::GraphicsPipelineInformation graphicsPipelineInformation{
		    "shader.vert.spv",
		    "shader.frag.spv",
		    *clearRenderPass,
		    *descriptorSetLayout,
		    vkx::Vertex::getBindingDescription(),
		    vkx::Vertex::getAttributeDescriptions()};

		const auto graphicsPipeline = device.createGraphicsPipeline(graphicsPipelineInformation);

		vkx::GraphicsPipelineInformation highlightGraphicsPipelineInformation{
		    "highlight.vert.spv",
		    "highlight.frag.spv",
		    *clearRenderPass,
		    *highlightDescriptorSetLayout,
		    getBindingDescription(),
		    getAttributeDescriptions()};

		const auto highlightGraphicsPipeline = device.createGraphicsPipeline(highlightGraphicsPipelineInformation);
		const auto commandSubmitter = device.createCommandSubmitter();
		constexpr std::uint32_t chunkDrawCommandAmount = 1;
		constexpr std::uint32_t highlightDrawCommandAmount = 1;

		constexpr std::uint32_t drawCommandAmount = chunkDrawCommandAmount + highlightDrawCommandAmount;
		constexpr std::uint32_t secondaryDrawCommandAmount = 4;
		const auto drawCommands = commandSubmitter->allocateDrawCommands(drawCommandAmount);
		const auto secondaryDrawCommands = commandSubmitter->allocateSecondaryDrawCommands(secondaryDrawCommandAmount);
		const auto syncObjects = device.createSyncObjects();

		vkx::VoxelChunk<16> chunk{{0, 0, 0}};
		for (int j = 0; j < 10; j++) {
			for (int k = 0; k < 4; k++) {
				for (int i = 0; i < 14; i++) {
					chunk.set(j, k, i, vkx::Voxel::Air);
				}
			}
		}

		chunk.greedy();

		vkx::VoxelChunk<16> chunk1{{1, 0, 0}};
		chunk1.greedy();

		vkx::VoxelChunk<16> chunk2{{0, 0, 1}};
		chunk2.greedy();

		vkx::VoxelChunk<16> chunk3{{1, 0, 1}};
		chunk3.greedy();

		vkx::Mesh mesh{chunk.vertices, chunk.indices, allocator};
		mesh.indexCount = std::distance(chunk.indices.begin(), chunk.indexIter);

		vkx::Mesh mesh1{chunk1.vertices, chunk1.indices, allocator};
		mesh1.indexCount = std::distance(chunk1.indices.begin(), chunk1.indexIter);

		vkx::Mesh mesh2{chunk2.vertices, chunk2.indices, allocator};
		mesh2.indexCount = std::distance(chunk2.indices.begin(), chunk2.indexIter);

		vkx::Mesh mesh3{chunk3.vertices, chunk3.indices, allocator};
		mesh3.indexCount = std::distance(chunk3.indices.begin(), chunk3.indexIter);

		const vkx::Texture texture{"a.jpg", device, allocator, commandSubmitter};

		std::vector<glm::vec3> vertices = {
		    {0.0f, 0.0f, 0.0f},
		    {0.0f, 1.0f, 0.0f},
		    {1.0f, 1.0f, 0.0f},
		    {1.0f, 0.0f, 0.0f},

		    {1.0f, 0.0f, 0.0f},
		    {1.0f, 1.0f, 0.0f},
		    {1.0f, 1.0f, 1.0f},
		    {1.0f, 0.0f, 1.0f},

		    {1.0f, 0.0f, 1.0f},
		    {1.0f, 1.0f, 1.0f},
		    {0.0f, 1.0f, 1.0f},
		    {0.0f, 0.0f, 1.0f},

		    {0.0f, 0.0f, 1.0f},
		    {0.0f, 1.0f, 1.0f},
		    {0.0f, 1.0f, 0.0f},
		    {0.0f, 0.0f, 0.0f},

		    {0.0f, 0.0f, 0.0f},
		    {1.0f, 0.0f, 0.0f},
		    {1.0f, 0.0f, 1.0f},
		    {0.0f, 0.0f, 1.0f},

		    {0.0f, 1.0f, 0.0f},
		    {0.0f, 1.0f, 1.0f},
		    {1.0f, 1.0f, 1.0f},
		    {1.0f, 1.0f, 0.0f},
		};

		std::vector<std::uint32_t> indices = {
		    0, 1, 2, 2, 3, 0,
		    4, 5, 6, 6, 7, 4,
		    8, 9, 10, 10, 11, 8,
		    12, 13, 14, 14, 15, 12,
		    16, 17, 18, 18, 19, 16,
		    20, 21, 22, 22, 23, 20};

		vkx::Mesh highlightMesh(vertices, indices, allocator);

		auto mvpBuffers = allocator->allocateUniformBuffers(vkx::MVP{});
		auto lightBuffers = allocator->allocateUniformBuffers(vkx::DirectionalLight{});
		auto materialBuffers = allocator->allocateUniformBuffers(vkx::Material{});

		auto highlightMVPBuffers = allocator->allocateUniformBuffers(vkx::MVP{});

		const std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
		const vk::DescriptorSetAllocateInfo allocInfo(*descriptorPool, layouts);

		const auto descriptorSets = device->allocateDescriptorSets(allocInfo);

		for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			const std::array descriptorWrites = {
			    mvpBuffers[i].createWriteDescriptorSet(descriptorSets[i], 0),
			    texture.createWriteDescriptorSet(descriptorSets[i], 1),
			    lightBuffers[i].createWriteDescriptorSet(descriptorSets[i], 2),
			    materialBuffers[i].createWriteDescriptorSet(descriptorSets[i], 3),
			};

			device->updateDescriptorSets(descriptorWrites, {});
		}

		const std::vector<vk::DescriptorSetLayout> highlightLayouts(MAX_FRAMES_IN_FLIGHT, *highlightDescriptorSetLayout);
		const vk::DescriptorSetAllocateInfo highlightAllocInfo(*highlightDescriptorPool, highlightLayouts);

		const auto highlightDescriptorSets = device->allocateDescriptorSets(highlightAllocInfo);

		for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			const std::array descriptorWrites = {
			    highlightMVPBuffers[i].createWriteDescriptorSet(highlightDescriptorSets[i], 0),
			};

			device->updateDescriptorSets(descriptorWrites, {});
		}

		auto proj = glm::perspective(70.0f, 640.0f / 480.0f, 0.1f, 100.0f);

		auto highlightModel = glm::mat4(1.0f);

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

			auto& highlightMVPBuffer = highlightMVPBuffers[currentFrame];
			highlightMVPBuffer->model = highlightModel;
			highlightMVPBuffer->view = mvpBuffer->view;
			highlightMVPBuffer->proj = mvpBuffer->proj;

			const auto& syncObject = syncObjects[currentFrame];
			syncObject.waitForFence();
			auto [result, imageIndex] = swapchain->acquireNextImage(device, syncObject);

			if (result == vk::Result::eErrorOutOfDateKHR) {
				int newWidth;
				int newHeight;
				SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
				while (newWidth == 0 || newHeight == 0) {
					SDL_Vulkan_GetDrawableSize(window, &newWidth, &newHeight);
					SDL_WaitEvent(nullptr);
				}

				device->waitIdle();

				swapchain = device.createSwapchain(window, allocator);

				swapchain->createFramebuffers(device, *clearRenderPass);
				continue;
			} else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
				throw std::runtime_error("Failed to acquire next image.");
			}

			mvpBuffer.mapMemory();
			lightBuffer.mapMemory();
			materialBuffer.mapMemory();
			highlightMVPBuffer.mapMemory();

			syncObject.resetFence();

			const vkx::DrawInfo chunkDrawInfo = {
			    *clearRenderPass,
			    *swapchain->framebuffers[imageIndex],
			    swapchain->extent,
			    *graphicsPipeline->pipeline,
			    *graphicsPipeline->layout,
			    descriptorSets[currentFrame],
			    {mesh.vertex->object, mesh1.vertex->object, mesh2.vertex->object, mesh3.vertex->object},
			    {mesh.index->object, mesh1.index->object, mesh2.index->object, mesh3.index->object},
			    {static_cast<std::uint32_t>(mesh.indexCount), static_cast<std::uint32_t>(mesh1.indexCount), static_cast<std::uint32_t>(mesh2.indexCount), static_cast<std::uint32_t>(mesh3.indexCount)}};

			const vkx::DrawInfo highlightDrawInfo = {
			    *loadRenderPass,
			    *swapchain->framebuffers[imageIndex],
			    swapchain->extent,
			    *highlightGraphicsPipeline->pipeline,
			    *highlightGraphicsPipeline->layout,
			    highlightDescriptorSets[currentFrame],
			    {highlightMesh.vertex->object},
			    {highlightMesh.index->object},
			    {static_cast<std::uint32_t>(highlightMesh.indexCount)}};

			const vk::CommandBuffer* begin = &drawCommands[currentFrame * drawCommandAmount];

			const vk::CommandBuffer* chunkBegin = begin;

			const vk::CommandBuffer* highlightBegin = chunkBegin + chunkDrawCommandAmount;

			const vk::CommandBuffer* secondaryBegin = &secondaryDrawCommands[currentFrame * secondaryDrawCommandAmount];

			commandSubmitter->recordSecondaryDrawCommands(chunkBegin, chunkDrawCommandAmount, secondaryBegin, secondaryDrawCommandAmount, chunkDrawInfo);

			commandSubmitter->recordPrimaryDrawCommands(highlightBegin, highlightDrawCommandAmount, highlightDrawInfo);

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

				device->waitIdle();

				swapchain = device.createSwapchain(window, allocator);

				swapchain->createFramebuffers(device, *clearRenderPass);
			} else if (result != vk::Result::eSuccess) {
				throw std::runtime_error("Failed to present.");
			}

			currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

			vkx::RaycastResult raycastResult{};
			auto a = [&chunk](const auto& b) {
				return chunk.at(b) != vkx::Voxel::Air;
			};
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
				case SDL_MOUSEMOTION: {
					camera.updateMouse({-event.motion.xrel, -event.motion.yrel});
					const auto origin = glm::vec3(chunk.normalizedPosition) - camera.position;

					raycastResult = vkx::raycast(origin, camera.front, 4.0f, a);
					if (raycastResult.success) {
						highlightModel = glm::translate(glm::mat4(1.0f), glm::vec3(chunk.normalizedPosition - raycastResult.hitPos) + glm::vec3(-1.0f));
					}
				} break;
				case SDL_KEYDOWN: {
					camera.updateKey(event.key.keysym.sym);
					const auto origin = glm::vec3(chunk.normalizedPosition) - camera.position;
					const vkx::AABB box{glm::vec3{0}, glm::vec3{1}};
					const auto collision = vkx::handleCollision(box, origin, glm::vec3{0, 1, 0}, 1.0f, a);
					if (collision.success) {
						SDL_Log("Colliding");
					}
				} break;
				case SDL_KEYUP:
					camera.direction = glm::vec3(0);
					break;
				case SDL_MOUSEBUTTONDOWN: {
					const auto origin = glm::vec3(chunk.normalizedPosition) - camera.position;
					raycastResult = vkx::raycast(origin, camera.front, 4.0f, a);
					if (raycastResult.success) {
						chunk.set(raycastResult.hitPos, vkx::Voxel::Air);
						chunk.greedy();
						mesh.vertex->mapMemory(chunk.vertices);
						mesh.index->mapMemory(chunk.indices);
						mesh.indexCount = std::distance(chunk.indices.begin(), chunk.indexIter);
					}
				} break;
				default:
					break;
				}
			}
		}

		device->waitIdle();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
