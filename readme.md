# Vulkan Voxel Project - VKX

VKX is a 2D voxel engine that uses the Vulkan rendering API. Its primary purpose is to create a game with inspiration from Minecraft, Valheim, Terraria, and Starbound. Its primary target platform is desktop gaming and mobile gaming. Thus this will compile for the x86 and ARM architectures.

### Libraries used
1. [Vulkan](https://www.vulkan.org/) Rendering API by Khronos Group
2. [Vulkan Memory Allocator - VMA](https://gpuopen.com/vulkan-memory-allocator/) by AMDs GPU Open
3. [GLM](https://github.com/g-truc/glm) by g-truc
4. [STB](https://github.com/nothings/stb) Image by nothings

## Collision Detection
Implement swept AABB collisions from player to voxel world. The code for this will be located in world space manipulation related files. There is already raycasting code there will just have to be a modified algorithm to raycast an AABB object.

## Application Tickrate
Implement a [delta timer](https://en.wikipedia.org/wiki/Delta_timing) and a tickrate state machine to ensure proper physics within the game world. The tickrate will ensure entities and anything effected by tickrate will behave properly. This also allows for the ability to calculate how long a player has been gone from a chunk to allow for natural change. A great reference for this is [Fix Your Timestep!](https://www.gafferongames.com/post/fix_your_timestep/) or the [stackoverflow question](https://stackoverflow.com/questions/59441699/gaffer-on-games-timestep-stdchrono-implementation) about the implementation of the article in C++.

## User Interface
Implement a simple UI stating some basic information about what is going on in the VKX application. Things such as current FPS, application tickrate, memory allocated, and potentially the ability to refresh chunks. This UI will use the Dear IMGUI library as a backend.

## Monolithic Files
- Combine aabb.cpp and raycast.cpp into a file that pertains to world space manipulation and interaction. Voxels will not be included here as this file should be for graphical and physics manipulation not the actual voxel world itself.
- Water down the pipeline and swapchain objects. Then move them to the same file. Potential candidate for objects to be moved to is renderer.hpp file.
- Move swapchain info, queue config, and sync objects structures to renderer.hpp file.
- Move model.hpp and image.hpp and uniform_buffer.hpp into renderer.hpp. Reasoning is they are all similar objects.