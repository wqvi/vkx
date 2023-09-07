# **V**ul**k**an Vo**x**el - VKX
This project aims to be a two dimensional voxel exploration simulator.

## Build instructions
In the base directory of the repository just run
```bash
cmake -S . -B build
cmake --build build
```

## Example usage
Once cmake is invoked and built a vkx executable will be present in the build directory. Current versions of vkx have issues with resource resolution. Therefore it is a requirement to run vkx from the base directory of the repository.
```bash
./build/vkx
```

### Libraries used
- [Vulkan](https://www.vulkan.org/)
- [shaderc](https://github.com/google/shaderc)
- [Vulkan Memory Allocator - VMA](https://gpuopen.com/vulkan-memory-allocator/)
- [glm](https://github.com/g-truc/glm)
- [stb](https://github.com/nothings/stb)
