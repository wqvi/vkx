# Archive
This was my crash course on game engine development. I've since abandoned it. The engine/game thingy(?) garnered too much technical debt and I found myself swamped. A couple things I have learned since then is that an Entity Component system would help. Not to mention raycasting from the camera position would need inverse matrices, aka camera space to normalized device coordinates (NDC). The list goes on. I could do a post-mortem analysis on this but that would involve a considerable amount of time that I don't have right now. Feel free to ask me any questions about this I had a lot of fun making it.

I am creating something new, a successor of this. Written in C99 and OpenGL instead of Vulkan. This new one uses minimal amounts of libraries and I have found myself to have gotten much farther with less feature bloat.

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
