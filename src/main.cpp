#include "vkx/camera.hpp"
#include "vkx/renderer/core/renderer_base.hpp"
#include "vkx/renderer/core/renderer_types.hpp"
#include "vkx/renderer/core/vertex.hpp"
#include "vkx/renderer/model.hpp"
#include "vkx/renderer/uniform_buffer.hpp"
#include "vkx/voxels/voxels.hpp"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>
#include <cstddef>
#include <cstdint>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vk_mem_alloc.h>
#include <vkx/vkx.hpp>
#include <vulkan/vulkan_core.h>

int main(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failure to initialize SDL2: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	const auto windowFlags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;

	SDL_Window* const window = SDL_CreateWindow("Jewelry", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, windowFlags);

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

		vkx::RendererBase renderer(window);
		const auto device = renderer.createDevice();
		const auto allocator = device->createAllocator();
		// const auto swapchain = device->createSwapchain(window, allocator);
		// const auto graphicsPipeline = {1, 2, 3};

		vkx::VoxelChunk<16> chunk({0, 0, 0});
		chunk.greedy();

		vkx::Model model({}, renderer.allocateTexture("a.jpg"), {glm::vec3(0.2f), 100.0f});
		// model.mesh.indexCount = chunk.vertexCount;

		auto mesh = renderer.allocateMesh(chunk.ve, chunk.in);
		mesh.indexCount = std::distance(chunk.in.begin(), chunk.indexIter);
		
		auto mvpBuffers = renderer.createBuffers(vkx::MVP{});
		auto lightBuffers = renderer.createBuffers(vkx::DirectionalLight{});
		auto materialBuffers = renderer.createBuffers(vkx::Material{});
		renderer.createDescriptorSets(mvpBuffers, lightBuffers, materialBuffers, model.texture);

		auto proj = glm::perspective(70.0f, 640.0f / 480.0f, 0.1f, 100.0f);
		proj[1][1] *= -1.0f;

		SDL_Event event{};
		bool isRunning = true;
		SDL_ShowWindow(window);
		while (isRunning) {
			auto currentFrame = renderer.getCurrentFrameIndex();

			camera.position += camera.direction * 0.0001f;

			auto& mvpBuffer = mvpBuffers[currentFrame];
			mvpBuffer->model = model.getModelMatrix();
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
			materialBuffer->specularColor = model.material.specularColor;
			materialBuffer->shininess = model.material.shininess;

			// TODO whatever this is lol
			renderer.drawFrame(mvpBuffer, lightBuffer, materialBuffer,
					   mesh.vertex->object, mesh.index->object,
					   static_cast<std::uint32_t>(mesh.indexCount),
					   currentFrame);

			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_QUIT:
					isRunning = false;
					break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
						renderer.framebufferResized = true;
						proj = glm::perspective(70.0f, static_cast<float>(event.window.data1) / static_cast<float>(event.window.data2), 0.1f, 100.0f);
						proj[1][1] *= -1.0f;
					}
					break;
				case SDL_MOUSEMOTION:
					camera.updateMouse({event.motion.xrel, -event.motion.yrel});
					break;
				case SDL_KEYDOWN:
					camera.updateKey(event.key.keysym.sym);
					break;
				case SDL_KEYUP:
					camera.updateKey(0);
				default:
					break;
				}
			}
		}

		renderer.waitIdle();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
