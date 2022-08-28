template <class T>
std::shared_ptr<vkx::Allocation<vk::Buffer>> vkx::Allocator::allocateBuffer(const std::vector<T>& data, vk::BufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) const {
	const vk::BufferCreateInfo bufferCreateInfo({}, data.size() * sizeof(T), bufferUsage, vk::SharingMode::eExclusive);

	const VmaAllocationCreateInfo allocationCreateInfo = createAllocationInfo(flags, memoryUsage);

	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo = {};
	if (vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory resources.");
	}

	std::memcpy(allocationInfo.pMappedData, data.data(), allocationInfo.size);

	return std::make_shared<vkx::Allocation<vk::Buffer>>(vk::Buffer(buffer), allocation, allocationInfo, allocator);
}

template <class T, std::size_t size>
std::shared_ptr<vkx::Allocation<vk::Buffer>> vkx::Allocator::allocateBuffer(const std::array<T, size>& data, vk::BufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) const {
	const vk::BufferCreateInfo bufferCreateInfo({}, sizeof(T) * size, bufferUsage, vk::SharingMode::eExclusive);

	const VmaAllocationCreateInfo allocationCreateInfo = createAllocationInfo(flags, memoryUsage);

	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo = {};
	if (vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory resources.");
	}

	std::memcpy(allocationInfo.pMappedData, data.data(), allocationInfo.size);

	return std::make_shared<vkx::Allocation<vk::Buffer>>(vk::Buffer(buffer), allocation, allocationInfo, allocator);
}

template <class T>
std::shared_ptr<vkx::Allocation<vk::Buffer>> vkx::Allocator::allocateBuffer(const T& data, vk::BufferUsageFlags bufferUsage, VmaAllocationCreateFlags flags, VmaMemoryUsage memoryUsage) const {
	const vk::BufferCreateInfo bufferCreateInfo({}, sizeof(T), bufferUsage, vk::SharingMode::eExclusive);

	const VmaAllocationCreateInfo allocationCreateInfo = createAllocationInfo(flags, memoryUsage);

	VkBuffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo = {};
	if (vmaCreateBuffer(allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), &allocationCreateInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory resources.");
	}

	std::memcpy(allocationInfo.pMappedData, &data, allocationInfo.size);

	return std::make_shared<vkx::Allocation<vk::Buffer>>(vk::Buffer(buffer), allocation, allocationInfo, allocator);
}

template <class T>
std::vector<vkx::UniformBuffer<T>> vkx::Allocator::allocateUniformBuffers(const T& value) const {
	std::vector<vkx::UniformBuffer<T>> buffers;
	buffers.reserve(MAX_FRAMES_IN_FLIGHT);
	for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		buffers.emplace_back(value, allocateBuffer(value, vk::BufferUsageFlagBits::eUniformBuffer));
	}
	return buffers;
}

template <class T>
template <class K>
void vkx::Allocation<T>::mapMemory(const std::vector<K>& memory) const {
	std::memcpy(allocationInfo.pMappedData, memory.data(), allocationInfo.size);
}

template <class T>
template <class K, std::size_t size>
void vkx::Allocation<T>::mapMemory(const std::array<K, size>& memory) const {
	std::memcpy(allocationInfo.pMappedData, memory.data(), allocationInfo.size);
}

template <>
inline vkx::Allocation<vk::Image>::~Allocation() {
	vmaDestroyImage(allocator, static_cast<VkImage>(object), allocation);
}

template <>
inline vkx::Allocation<vk::Buffer>::~Allocation() {
	vmaDestroyBuffer(allocator, static_cast<VkBuffer>(object), allocation);
}