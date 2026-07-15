#include <iostream>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "Renderer2D.h"
#include "Material.h"

int main()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	glm::ivec2 size{ 800, 600 };
	SDL_Window* window = SDL_CreateWindow("Hello WebGPU", size.x, size.y, NULL);

	wgpu::Renderer2D renderer{};
	if (!renderer.Init(window)) return -1;

	renderer.SetClearColor(0.f, 0.f, 0.f);

	wgpu::Shader shader{ *renderer.GetContext(), "resources/SolidColorMult3X.wgsl" };

	wgpu::BindGroupLayout layout{};
	layout.AddUniformEntry<float>(0, wgpu::BindingVisibility::Both);
	layout.AddUniformEntry<float>(1, wgpu::BindingVisibility::Both);
	layout.AddUniformEntry<float>(2, wgpu::BindingVisibility::Both);
	layout.ConfirmLayout(*shader.GetGraphicsContext());

	wgpu::Pipeline pipeline{ shader, &layout };
	wgpu::Material material{ pipeline };

	material.SetUniform(0, 1.f);
	material.SetUniform(1, 0.5f);
	material.SetUniform(2, 2.f);

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
	v10.color.r = 1.f;

	Vertex v11{  };
	v11.position = glm::vec3{ -0.05f, 0.5f, 0.f };
	v11.color.g = 1.f;

	Vertex v12{  };
	v12.position = glm::vec3{ -0.55f, 0.5f, 0.f };
	v12.color.b = 1.f;

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
		}
		renderer.EndFrame();
		renderer.Render();
	}
	renderer.Quit();

	SDL_DestroyWindow(window);
	SDL_Quit();
}