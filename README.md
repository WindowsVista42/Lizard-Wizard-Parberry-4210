# Lizard-Wizard-Parberry-4210
A game made for CSCE 4210 at UNT.  

## Project Overview
For this project I programmed a 3d deferred renderer with support for models, lighting, textures, bloom, tonemapping, blurring, tinting, and desaturation effects.
This renderer also supported a 'debug' renderer mode that used DirectXTK12's PrimitiveBatch to build, batch, and push vertex data to the gpu to be rendered. This method of calculating vertex transformations on the CPU is a very poor usage of graphics hardware so it was only used for debugging purposes while developing and did not make its way into the final game.
The 3d renderer however, uses gpu memory and is deferred in nature. What this means is that vertex and index buffers and textures are all loaded onto the gpu into the corresponding buffer type, and we use constant buffers to push instance-specific data.The renderer uses MRT to render to a position, normal, and color texture that is then used on a lighting pass and any other screen space effect passes. The benefit of this approach is that it moves calculations such as lighting from a per-triangle basis to a per-pixel basis helping to significantly reduce calculation for only a few tradeoffs.
The tradeoffs of this approach are that anti-aliasing techniques such as MSAA are complex and slow to implement compared to a forward or forward+ renderer. As well, transparency would not work with the current approach, and significant changes would have to be made to handle transparent items.
This renderer is also very unoptimized and doesn't even use basic things such as frustum culling, but the performance was high enough for our use-case that this did not need to be implemented.  

I programmed an entity component system to go along with this, that was used as our primary data storage method. This entity component system allowed us to dynamically add and remove components from entities while maintaining high performance. This system also allowed us to iterate over components in a partial manner, meaning that we could selectively load only the Position and Velocity components from an entity that might also have a Model, Rotation, and Scale component.
This type of system helps to better utilize cpu cache and system memory when iterating over large numbers of entities, however, our system had some major flaws that, according to some napkin math, probably landed it somewhere in the range of 10x slower than it could have been.
One way this system was used in practice however, was to control the duration of projectiles and particles. We would attach a timer component to them when spawning, iterate through all timers in the game and update them (including ones not related to projectiles or particles), and when the timer ran out we would detach the timer and despawn the entity. Note however, that the spawning and despawning of entities did not allocate any memory, rather, we allocated all memory up-front and used a caching system to quickly add and remove entities.  

I programmed the procedural generation system which takes a 2d grid, fills out spots to either have a room or not have a room, pathfinds to find a path to the boss room, if a path is not found, it will recursively iterate until a path is built, then it reconstructs 3d meshes from the 2d grid.
To go with the procedural generation system, I also programmed A* pathfinding for NPC's and a custom algorithm similar to dijkstra's for finding the closest point to the end during map generation.
The reason I did not use A* for finding the closest point to the end is that it was practically gauranteed that there was not a path, and the extra overhead of maintaining a priority queue, among other things, was found to slow down procedural generation quite a bit.  

Lastly, I programmed some minor game systems such as a 3d map, player movement, mana, and health systems (excluding physics connections), a 3d UI system, and a simple fixed buffer linear allocator.

## Gameplay Video
The engine supports custom models and textures, however, I'm not an artist and I don't know how to texture, so everything uses a white texture and very basic models.
https://youtu.be/bE3m0kThij8
