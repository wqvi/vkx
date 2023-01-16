#pragma once

namespace vkx {
namespace alloc {
template <class T, class K, class U>
class ManagedDeleter {
private:
	T function{};
	K deleter{};

public:
	ManagedDeleter() = default;

	ManagedDeleter(T function, K deleter)
	    : function(function),
	      deleter(deleter) {}

	void operator()(U obj) const noexcept {
		if (obj != nullptr) {
			function(deleter, obj);
		}
	}
};

using VmaAllocationDeleter = vkx::alloc::ManagedDeleter<decltype(&vmaFreeMemory), VmaAllocator, VmaAllocation>;

using UniqueVmaAllocation = std::unique_ptr<std::remove_pointer_t<VmaAllocation>, VmaAllocationDeleter>;

using VmaPoolDeleter = vkx::alloc::ManagedDeleter<decltype(&vmaDestroyPool), VmaAllocator, VmaPool>;

using UniqueVmaPool = std::unique_ptr<std::remove_pointer_t<VmaPool>, VmaPoolDeleter>;

template <class T, class K>
class Deleter {
private:
	T function{};

public:
	Deleter() = default;

	Deleter(T function)
	    : function(function) {}

	void operator()(K obj) const noexcept {
		if (obj != nullptr) {
			function(obj);
		}
	}
};

using VmaAllocatorDeleter = Deleter<decltype(&vmaDestroyAllocator), VmaAllocator>;

using UniqueVmaAllocator = std::unique_ptr<std::remove_pointer_t<VmaAllocator>, VmaAllocatorDeleter>;

using SharedVmaAllocator = std::shared_ptr<std::remove_pointer_t<VmaAllocator>>;

using WeakVmaAllocator = std::weak_ptr<std::remove_pointer_t<VmaAllocator>>;
} // namespace alloc
} // namespace vkx