#include <iostream>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "Renderer2D.h"
#include "Material.h"
#include "Texture2D.h"
#include "Sampler.h"

int main()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	glm::ivec2 size{ 800, 600 };
	SDL_Window* window = SDL_CreateWindow("Hello WebGPU", size.x, size.y, SDL_WINDOW_RESIZABLE);

	wgpu::Renderer2D renderer{};
	if (!renderer.Init(window)) return -1;

	renderer.SetClearColor(0.05f, 0.05f, 0.05f);

	//wgpu::Shader shader{ *renderer.GetContext(), "resources/SolidColorMultRGBA.wgsl" };
	wgpu::Shader shader{ *renderer.GetContext(), "resources/BasicTextureAndSampler.wgsl" };

	wgpu::Texture2D texture{ *renderer.GetContext(), "resources/texture.jpg" };
	wgpu::Sampler sampler{ *renderer.GetContext(), wgpu::Sampler::Preset::Smooth };

	wgpu::BindGroupLayout layout{};
	layout.AddTextureEntry(0);
	layout.AddSamplerEntry(1);
	layout.ConfirmLayout(*shader.GetGraphicsContext());

	wgpu::Pipeline pipeline{ shader, &layout };
	wgpu::Material material{ pipeline };
	material.SetTexture(0, &texture);
	material.SetSampler(1, &sampler);

	Vertex v00{};	// bottom-left
	v00.position = glm::vec3{ -0.5f, -0.5f, 0.f };
	v00.color.r = 1.f;
	v00.uv = glm::vec2{ 0.f, 1.f };

	Vertex v01{};	// bottom-right
	v01.position = glm::vec3{ 0.5f, -0.5f, 0.f };
	v01.color.g = 1.f;
	v01.uv = glm::vec2{ 1.f, 1.f };

	Vertex v02{};	// top-right
	v02.position = glm::vec3{ 0.5f, 0.5f, 0.f };
	v02.color.b = 1.f;
	v02.uv = glm::vec2{ 1.f, 0.f };

	Vertex v03{};	// top-left
	v03.position = glm::vec3{ -0.5f, 0.5f, 0.f };
	v03.color.g = 1.f;
	v03.uv = glm::vec2{ 0.f, 0.f };
	
	ColorF multiplier{ 1.f, 1.f, 1.f };
	material.SetUniform(0, multiplier);

	bool isRunning{ true };
	while (isRunning)
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			isRunning = event.type != SDL_EVENT_QUIT;

			switch (event.type)
			{
			case SDL_EVENT_WINDOW_RESIZED:
				renderer.Resize();
				break;

			default:
				break;
			}
		}

		renderer.BeginFrame();
		{
			renderer.Queue().PushTriangle(material, v00, v01, v02);
			renderer.Queue().PushTriangle(material, v00, v02, v03);
		}
		renderer.EndFrame();
		renderer.Render();
	}
	renderer.Quit();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}