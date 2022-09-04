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
}

auto createShaderBindings() {
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

	return std::vector{uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding, materialLayoutBinding};
}

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

auto createHighlightShaderBindings() noexcept {
	constexpr vk::DescriptorSetLayoutBinding uboLayoutBinding{
	    0,
	    vk::DescriptorType::eUniformBuffer,
	    1,
	    vk::ShaderStageFlagBits::eVertex,
	    nullptr};

	return std::vector{uboLayoutBinding};
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

	if (SDL_SetRelativeMouseMode(SDL_TRUE)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failure to capture mouse: %s", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	{
		const vkx::SDLWindow window{"Among Us", 640, 480};

		// vkx::Renderer renderer{window};

		// const auto* graphicsPipeline = renderer.attachPipeline({"shader.vert.spv",
		// 			 "shader.frag.spv",
		// 			 createShaderBindings(),
		// 			 vkx::Vertex::getBindingDescription(),
		// 			 vkx::Vertex::getAttributeDescriptions()});

		// const auto* highlightGraphicsPipeline = renderer.attachPipeline({"highlight.vert.spv",
		// 			 "highlight.frag.spv",
		// 			 createHighlightShaderBindings(),
		// 			 getBindingDescription(),
		// 			 getAttributeDescriptions()});

		vkx::Camera camera({0, 0, 0});

		const vkx::RendererBootstrap bootstrap{static_cast<SDL_Window*>(window)};
		const auto device = bootstrap.createDevice();
		const auto allocator = device.createAllocator();
		const auto commandSubmitter = device.createCommandSubmitter();
		const vkx::SwapchainInfo swapchainInfo{device};
		const auto clearRenderPass = device.createRenderPass(swapchainInfo.chooseSurfaceFormat().format, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp::eClear);
		const auto loadRenderPass = device.createRenderPass(swapchainInfo.chooseSurfaceFormat().format, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::AttachmentLoadOp::eLoad);
		auto swapchain = device.createSwapchain(static_cast<SDL_Window*>(window), *clearRenderPass, allocator);

		const std::vector poolSizes = {vkx::UNIFORM_BUFFER_POOL_SIZE, vkx::SAMPLER_BUFFER_POOL_SIZE, vkx::UNIFORM_BUFFER_POOL_SIZE, vkx::UNIFORM_BUFFER_POOL_SIZE};

		const std::vector highlightPoolSizes = {vkx::UNIFORM_BUFFER_POOL_SIZE};

		const auto descriptorSetLayout = createShaderDescriptorSetLayout(device);

		const auto highlightDescriptorSetLayout = createHighlightDescriptorSetLayout(device);

		const vkx::Texture texture{"a.jpg", device, allocator, commandSubmitter};

		const vkx::GraphicsPipelineInformation graphicsPipelineInformation{
		    "shader.vert.spv",
		    "shader.frag.spv",
		    createShaderBindings(),
		    vkx::Vertex::getBindingDescription(),
		    vkx::Vertex::getAttributeDescriptions(),
			{sizeof(vkx::MVP), sizeof(vkx::DirectionalLight), sizeof(vkx::Material)},
			{&texture}};

		const auto graphicsPipeline = device.createGraphicsPipeline(*clearRenderPass, allocator, graphicsPipelineInformation);

		const vkx::GraphicsPipelineInformation highlightGraphicsPipelineInformation{
		    "highlight.vert.spv",
		    "highlight.frag.spv",
		    createHighlightShaderBindings(),
		    getBindingDescription(),
		    getAttributeDescriptions(),
			{sizeof(vkx::MVP)},
			{}};

		const auto highlightGraphicsPipeline = device.createGraphicsPipeline(*clearRenderPass, allocator, highlightGraphicsPipelineInformation);

		constexpr std::uint32_t chunkDrawCommandAmount = 1;
		constexpr std::uint32_t highlightDrawCommandAmount = 1;

		constexpr std::uint32_t drawCommandAmount = chunkDrawCommandAmount + highlightDrawCommandAmount;
		constexpr std::uint32_t secondaryDrawCommandAmount = 4;
		const auto drawCommands = commandSubmitter.allocateDrawCommands(drawCommandAmount);
		const auto secondaryDrawCommands = commandSubmitter.allocateSecondaryDrawCommands(secondaryDrawCommandAmount);
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

		const vkx::Mesh highlightMesh{vkx::CUBE_VERTICES, vkx::CUBE_INDICES, allocator};

		// const std::vector<vkx::DrawInfoTest> drawInfos = {
		// 	{vk::CommandBufferLevel::eSecondary, graphicsPipeline, {&mesh, &mesh1, &mesh2, &mesh3}},
		// 	{vk::CommandBufferLevel::eSecondary, highlightGraphicsPipeline, {&highlightMesh}}
		// };

		// renderer.createDrawCommands(drawInfos);

		// auto mvpBuffers = allocator.allocateUniformBuffers(sizeof(vkx::MVP));
		// auto lightBuffers = allocator.allocateUniformBuffers(sizeof(vkx::DirectionalLight));
		// auto materialBuffers = allocator.allocateUniformBuffers(sizeof(vkx::Material));

		// auto highlightMVPBuffers = allocator.allocateUniformBuffers(sizeof(vkx::MVP));

		auto mvpBuffers = graphicsPipeline.getUniformByIndex(0);
		auto lightBuffers = graphicsPipeline.getUniformByIndex(1);
		auto materialBuffers = graphicsPipeline.getUniformByIndex(2);

		auto highlightMVPBuffers = graphicsPipeline.getUniformByIndex(0);

		// graphicsPipeline.updateDescriptorSets([&mvpBuffers, &texture, &lightBuffers, &materialBuffers](std::size_t i) -> std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>> {
		// 	return {mvpBuffers[i].createDescriptorBufferInfo(),
		// 		texture.createDescriptorImageInfo(),
		// 		lightBuffers[i].createDescriptorBufferInfo(),
		// 		materialBuffers[i].createDescriptorBufferInfo()};
		// });

		// highlightGraphicsPipeline.updateDescriptorSets([&highlightMVPBuffers](std::size_t i) -> std::vector<std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo>> {
		// 	return {highlightMVPBuffers[i].createDescriptorBufferInfo()};
		// });

		auto proj = glm::perspective(70.0f, 640.0f / 480.0f, 0.1f, 100.0f);

		auto highlightModel = glm::mat4(1.0f);

		std::uint32_t currentFrame = 0;
		SDL_Event event{};
		bool isRunning = true;
		bool framebufferResized = false;
		window.show();
		while (isRunning) {
			camera.position += camera.direction * 0.001f;

			auto& mvpBuffer = mvpBuffers[currentFrame];
			auto mvp = vkx::MVP{glm::mat4(1.0f), camera.viewMatrix(), proj};

			auto& lightBuffer = lightBuffers[currentFrame];
			auto light = vkx::DirectionalLight{
			    glm::vec3(1.0f, 3.0f, 1.0f),
			    camera.position,
			    glm::vec4(1.0f, 1.0f, 1.0f, 0.2f),
			    glm::vec3(1.0f, 1.0f, 1.0f),
			    glm::vec3(1.0f, 1.0f, 1.0f),
			    1.0f,
			    0.09f,
			    0.032f};

			auto& materialBuffer = materialBuffers[currentFrame];
			auto material = vkx::Material{glm::vec3(0.2f), 100.0f};

			auto& highlightMVPBuffer = highlightMVPBuffers[currentFrame];
			auto highlightMVP = vkx::MVP{highlightModel, mvp.view, mvp.proj};

			mvpBuffer.mapMemory(mvp);
			lightBuffer.mapMemory(light);
			materialBuffer.mapMemory(material);
			highlightMVPBuffer.mapMemory(highlightMVP);

			const auto& syncObject = syncObjects[currentFrame];
			syncObject.waitForFence();
			auto [result, imageIndex] = swapchain.acquireNextImage(device, syncObject);

			if (result == vk::Result::eErrorOutOfDateKHR) {
				auto [newWidth, newHeight] = window.getSize();
				while (newWidth == 0 || newHeight == 0) {
					std::tie(newWidth, newHeight) = window.getSize();
					SDL_WaitEvent(nullptr);
				}

				device->waitIdle();

				swapchain = device.createSwapchain(static_cast<SDL_Window*>(window), *clearRenderPass, allocator);
				continue;
			} else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
				throw std::runtime_error("Failed to acquire next image.");
			}

			syncObject.resetFence();

			const vkx::DrawInfo chunkDrawInfo = {
			    imageIndex,
			    currentFrame,
			    &swapchain,
			    &graphicsPipeline,
			    *clearRenderPass,
			    {mesh.vertex->object, mesh1.vertex->object, mesh2.vertex->object, mesh3.vertex->object},
			    {mesh.index->object, mesh1.index->object, mesh2.index->object, mesh3.index->object},
			    {static_cast<std::uint32_t>(mesh.indexCount), static_cast<std::uint32_t>(mesh1.indexCount), static_cast<std::uint32_t>(mesh2.indexCount), static_cast<std::uint32_t>(mesh3.indexCount)}};

			const vkx::DrawInfo highlightDrawInfo = {
			    imageIndex,
			    currentFrame,
			    &swapchain,
			    &highlightGraphicsPipeline,
			    *loadRenderPass,
			    {highlightMesh.vertex->object},
			    {highlightMesh.index->object},
			    {static_cast<std::uint32_t>(highlightMesh.indexCount)}};

			const vk::CommandBuffer* begin = &drawCommands[currentFrame * drawCommandAmount];

			const vk::CommandBuffer* chunkBegin = begin;

			const vk::CommandBuffer* highlightBegin = chunkBegin + chunkDrawCommandAmount;

			const vk::CommandBuffer* secondaryBegin = &secondaryDrawCommands[currentFrame * secondaryDrawCommandAmount];

			commandSubmitter.recordSecondaryDrawCommands(chunkBegin, chunkDrawCommandAmount, secondaryBegin, secondaryDrawCommandAmount, chunkDrawInfo);

			commandSubmitter.recordPrimaryDrawCommands(highlightBegin, highlightDrawCommandAmount, highlightDrawInfo);

			commandSubmitter.submitDrawCommands(begin, drawCommandAmount, syncObject);

			result = commandSubmitter.presentToSwapchain(swapchain, imageIndex, syncObject);

			if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
				framebufferResized = false;

				auto [newWidth, newHeight] = window.getSize();
				while (newWidth == 0 || newHeight == 0) {
					std::tie(newWidth, newHeight) = window.getSize();
					SDL_WaitEvent(nullptr);
				}

				device->waitIdle();

				swapchain = device.createSwapchain(static_cast<SDL_Window*>(window), *clearRenderPass, allocator);
			} else if (result != vk::Result::eSuccess) {
				throw std::runtime_error("Failed to present.");
			}

			currentFrame = (currentFrame + 1) % vkx::MAX_FRAMES_IN_FLIGHT;

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
						int width = event.window.data1;
						int height = event.window.data2;
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

	SDL_Quit();

	return EXIT_SUCCESS;
}
