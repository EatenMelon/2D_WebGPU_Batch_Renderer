#include <iostream>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "Renderer2D.h"
#include "Material.h"
#include "Texture2D.h"

int main()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	glm::ivec2 size{ 800, 600 };
	SDL_Window* window = SDL_CreateWindow("Hello WebGPU", size.x, size.y, NULL);

	wgpu::Renderer2D renderer{};
	if (!renderer.Init(window)) return -1;

	renderer.SetClearColor(0.05f, 0.05f, 0.05f);

	wgpu::Shader shader{ *renderer.GetContext(), "resources/SolidColorMultRGBA.wgsl" };

	wgpu::BindGroupLayout layout{};
	layout.AddUniformEntry<ColorF>(0, wgpu::BindingVisibility::Both);
	layout.ConfirmLayout(*shader.GetGraphicsContext());

	wgpu::Pipeline pipeline{ shader, &layout };
	wgpu::Material material{ pipeline };

	wgpu::Texture2D texture0{ *renderer.GetContext(), "resources/texture.png" };
	wgpu::Texture2D texture1{ *renderer.GetContext(), "resources/texture.jpg" };

	// triangle a
	Vertex v00{};
	v00.position = glm::vec3{ -0.5f, -0.5f, 0.f };
	v00.color.r = 1.f;

	Vertex v01{};
	v01.position = glm::vec3{ 0.5f, -0.5f, 0.f };
	v01.color.g = 1.f;

	Vertex v02{};
	v02.position = glm::vec3{ 0.f, 0.5f, 0.f };
	v02.color.b = 1.f;

	// triangle b
	Vertex v10{  };
	v10.position = glm::vec3{ -0.55f, -0.5f, 0.f };
	v10.color.g = 1.f;

	Vertex v11{  };
	v11.position = glm::vec3{ -0.05f, 0.5f, 0.f };
	v11.color.b = 1.f;

	Vertex v12{  };
	v12.position = glm::vec3{ -0.55f, 0.5f, 0.f };
	v12.color.r = 1.f;

	ColorF multiplier{};
	bool isRunning{ true };
	while (isRunning)
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			isRunning = event.type != SDL_EVENT_QUIT;
		}

		renderer.BeginFrame();
		{
			renderer.Queue().PushTriangle(material, v00, v01, v02);
			renderer.Queue().PushTriangle(material, v10, v11, v12);
			material.SetUniform(0, multiplier);

			float time = SDL_GetTicks() / 1000.f;
			multiplier.r = (1 + sinf(time)) * 2;
			multiplier.g = (1 + cosf(time)) * 2;
			multiplier.b = (1 + cosf(time) * sinf(time)) * 2;
		}
		renderer.EndFrame();

		renderer.Render();
	}
	renderer.Quit();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}