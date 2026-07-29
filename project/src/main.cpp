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
	wgpu::Shader shader{ renderer, "resources/CameraTest.wgsl" };

	wgpu::Texture2D texture{ *renderer.GetContext(), "resources/texture.jpg" };
	wgpu::Sampler sampler{ *renderer.GetContext(), wgpu::Sampler::Preset::Smooth };

	wgpu::BindGroupLayout layout{};
	layout.AddTextureEntry(0);
	layout.AddSamplerEntry(1);
	layout.AddUniformEntry<wgpu::CameraData>(2, wgpu::BindingVisibility::VertexShaderStage);
	layout.ConfirmLayout(*shader.GetRenderer()->GetContext());

	wgpu::Pipeline pipeline{ shader, &layout };
	wgpu::Material material{ pipeline };
	material.SetTexture(0, &texture);
	material.SetSampler(1, &sampler);

	wgpu::Vertex v00{};	// bottom-left
	v00.position = glm::vec3{ -0.25f, -0.25f, 0.1f };
	v00.color.r = 1.f;
	v00.uv = glm::vec2{ 0.f, 1.f };

	wgpu::Vertex v01{};	// bottom-right
	v01.position = glm::vec3{ 0.25f, -0.25f, 0.1f };
	v01.color.r = 1.f;
	v01.uv = glm::vec2{ 1.f, 1.f };

	wgpu::Vertex v02{};	// top-right
	v02.position = glm::vec3{ 0.25f, 0.25f, 0.1f };
	v02.color.r = 1.f;
	v02.uv = glm::vec2{ 1.f, 0.f };

	wgpu::Vertex v03{};	// top-left
	v03.position = glm::vec3{ -0.25f, 0.25f, 0.1f };
	v03.color.r = 1.f;
	v03.uv = glm::vec2{ 0.f, 0.f };

	wgpu::Vertex v10{};	// bottom-left
	v10.position = glm::vec3{ -0.5f, -0.5f, 0.f };
	v10.color.b = 1.f;
	v10.uv = glm::vec2{ 0.f, 1.f };

	wgpu::Vertex v11{};	// bottom-right
	v11.position = glm::vec3{ 0.5f, -0.5f, 0.f };
	v11.color.b = 1.f;
	v11.uv = glm::vec2{ 1.f, 1.f };

	wgpu::Vertex v12{};	// top-right
	v12.position = glm::vec3{ 0.5f, 0.5f, 0.f };
	v12.color.b = 1.f;
	v12.uv = glm::vec2{ 1.f, 0.f };

	wgpu::Vertex v13{};	// top-left
	v13.position = glm::vec3{ -0.5f, 0.5f, 0.f };
	v13.color.b = 1.f;
	v13.uv = glm::vec2{ 0.f, 0.f };

	auto camera{ std::make_shared<wgpu::Camera2D>() };

	//camera.SetZoom(-10.f);
	//camera.Focus(glm::vec2(0.25f, 0.f));

	renderer.SetCamera(camera);

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

			case SDL_EVENT_MOUSE_WHEEL:
				camera->Zoom(event.wheel.y / 100.f);
				//renderer.SetCamera(camera);
				break;

			case SDL_EVENT_MOUSE_MOTION:
				if ((event.motion.state & SDL_BUTTON_LEFT) != SDL_BUTTON_LEFT) break;

				glm::vec2 motion{ -event.motion.xrel, event.motion.yrel };

				camera->Move(motion / 1000.f * camera->GetZoom());
				//renderer.SetCamera(camera);
				break;

			default:
				break;
			}
		}

		renderer.BeginFrame();
		{
			renderer.Submit(material, v00, v01, v02);
			renderer.Submit(material, v00, v02, v03);

			renderer.Submit(material, v10, v11, v12);
			renderer.Submit(material, v10, v12, v13);
		}
		renderer.EndFrame();
		renderer.Render();
	}
	renderer.Quit();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}