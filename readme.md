# Vulkan Voxel Project - VKX

## Purpose
A game that draws inspiration from Minecraft and Starbound. Think of it like a 3D starbound or a Minecraft with a builtin advanced rocketry modpack. C++17 is employed due to Starbound's philosophy. Built from the ground up with a singular purpose, to be as good as possible at it's job. The Vulkan rendering API and AMDs Vulkan Memory Allocator library are used for finely tuned graphics to support the mass amounts of voxels needed to be rendered.

### Libraries used
1. Vulkan Rendering API by Khronos Group
2. Vulkan Memory Allocator (VMA) by AMDs GPU Open
3. GLM by g-truc
4. STB Image by nothings

## Collision Detection
Implement swept AABB collisions from player to voxel world. The code for this will be located in world space manipulation related files. There is already raycasting code there will just have to be a modified algorithm to raycast an AABB object.

## Application Tickrate
Implement a delta timer and a tickrate state machine to ensure proper physics within the game world. The tickrate will ensure entities and anything effected by tickrate will behave properly. This also allows for the ability to calculate how long a player has been gone from a chunk to allow for natural change.

## User Interface
Implement a simple UI stating some basic information about what is going on in the VKX application. Things such as current FPS, application tickrate, memory allocated, and potentially the ability to refresh chunks. This UI will use the Dear IMGUI library as a backend.

## Monolithic Files
- Combine aabb.cpp and raycast.cpp into a file that pertains to world space manipulation and interaction. Voxels will not be included here as this file should be for graphical and physics manipulation not the actual voxel world itself.
- Water down the pipeline and swapchain objects. Then move them to the same file. Potential candidate for objects to be moved to is renderer.hpp file.
- Move swapchain info, queue config, and sync objects structures to renderer.hpp file.
- Move model.hpp and image.hpp and uniform_buffer.hpp into renderer.hpp. Reasoning is they are all similar objects.