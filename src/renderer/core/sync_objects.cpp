#include <vkx/renderer/core/renderer_types.hpp>
#include <vkx/renderer/core/sync_objects.hpp>

vkx::SyncObjects::SyncObjects(VkDevice device)
    : device(device) {
	const VkSemaphoreCreateInfo semaphoreCreateInfo{
	    VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	    nullptr,
	    0};

	const VkFenceCreateInfo fenceCreateInfo{
	    VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	    nullptr,
	    VK_FENCE_CREATE_SIGNALED_BIT};

	vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore);
	vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore);
	vkCreateFence(device, &fenceCreateInfo, nullptr, &inFlightFence);
}

std::vector<vkx::SyncObjects> vkx::SyncObjects::createSyncObjects(VkDevice device) {
	std::vector<vkx::SyncObjects> objs;
	objs.resize(MAX_FRAMES_IN_FLIGHT);

	std::generate(objs.begin(), objs.end(), [&device]() { return vkx::SyncObjects{device}; });

	return objs;
}

void vkx::SyncObjects::waitForFence() const {
	vkWaitForFences(device, 1, &inFlightFence, true, UINT64_MAX);
}

void vkx::SyncObjects::resetFence() const {
	vkResetFences(device, 1, &inFlightFence);
}

void vkx::SyncObjects::destroy() const {
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroyFence(device, inFlightFence, nullptr);
}