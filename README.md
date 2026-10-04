# 2D WebGPU Batch Renderer

## About
This is a 2D batch renderer, using WebGPU and SDL3.
Mainly made for education purposes, but might also be used in future projects.

The renderer combines a render queue and state sorting (through materials) in order to minimize GPU state changes. Since I wanted to play around with some cool effects, the project also supports custom shaders with limitations and simple post processing effects.

## Dependencies
This project uses the following libraries:
 - webgpu-native
 - SDL3 and SDL3_image
 - GLM
 - Dear ImGui

## Plans for the future
This project is not finished, nor is it meant to be. I still plan on changing and cleaning some things up, while adding new features.


Here are some things I would like to add in the future:
 - **A custom namespace**, currently I'm using 'wgpu' which is kind of dangerous since WebGPU's C++ abstractions use this namespace as well. Even if i don't use this namespace I would like my own namespace.
 - **A cleaner CMake file**, which can link to more than Windows on a 64 bit systems, because currently you would have to change the cmake yourself and I would like to have FetchContent work at least.
 - **More exception handling and error catching**, I would like to make the library as robust as I can. This is also a good exercise for me since I neglect this part of programming sometimes.
 - **More support for shaders**, currently shaders can have bindings for uniforms, textures and samplers, but I'm sure the wgsl shader language has way more possibilities, like storage bindings for example.
 - **Mipmaps** for 2D textures
 - **Support for transparent windows**, this isn't too difficult i think, it's just fun to be able to have a transparent window.
 - **Writable textures**, I would like to be able to render to textures with the Renderer as well. I think could add more options for rendering and effects.

I'm sure more will come to mind when working further on this.
