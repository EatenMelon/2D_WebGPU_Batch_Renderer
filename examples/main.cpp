#include <iostream>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <Renderer2D.h>
#include <Camera2D.h>
#include <Material.h>
#include <Texture2D.h>

static void LoadQuad(std::vector<wgpu::Vertex3D>& vertices, const glm::vec3 position, const wgpu::ColorF& color)
{
	vertices.resize(4, wgpu::Vertex3D{});

	vertices[0].position = glm::vec3(-0.25f, 0.25f, 0.f) + position;
	vertices[0].color = color;
	vertices[0].uv = glm::vec2(0.f, 0.f);

	vertices[1].position = glm::vec3(-0.25f, -0.25f, 0.f) + position;
	vertices[1].color = color;
	vertices[1].uv = glm::vec2(0.f, 1.f);

	vertices[2].position = glm::vec3(0.25f, -0.25f, 0.f) + position;
	vertices[2].color = color;
	vertices[2].uv = glm::vec2(1.f, 1.f);
	
	vertices[3].position = glm::vec3(0.25f, 0.25f, 0.f) + position;
	vertices[3].color = color;
	vertices[3].uv = glm::vec2(1.f, 0.f);
}

int main()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	
	glm::ivec2 size{ 800, 600 };
	SDL_Window* window = SDL_CreateWindow("Welcome back, WebGPU", size.x, size.y, SDL_WINDOW_RESIZABLE);

	wgpu::Renderer2D renderer{ window };
	
	// render resources
	auto camera = renderer.GetCamera();
	camera->SetAspectRatio(800.f / 600.f);

	wgpu::Texture2D spriteA{ renderer, "resources/Sprite.png" };
	int binding = spriteA.GetMaterial()->GetUniformBinding<wgpu::CameraData>();
	spriteA.GetMaterial()->SetUniform<wgpu::CameraData>(binding, camera->GetCameraData());

	wgpu::Texture2D spriteB{ renderer, "resources/Sprite.png" };
	binding = spriteB.GetMaterial()->GetUniformBinding<wgpu::CameraData>();
	spriteB.GetMaterial()->SetUniform<wgpu::CameraData>(binding, camera->GetCameraData());

	std::vector<wgpu::Vertex3D> meshContainer{};

	// main loop
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
				{
					int binding = spriteA.GetMaterial()->GetUniformBinding<wgpu::CameraData>();
					spriteA.GetMaterial()->SetUniform<wgpu::CameraData>(binding, camera->GetCameraData());

					binding = spriteB.GetMaterial()->GetUniformBinding<wgpu::CameraData>();
					spriteB.GetMaterial()->SetUniform<wgpu::CameraData>(binding, camera->GetCameraData());
				}
				break;
			}
		}

		renderer.BeginFrame();
		{
			LoadQuad(meshContainer, glm::vec3(-0.35f, 0.f, 0.f), wgpu::ColorF(0.f, 1.f, 0.f));
			renderer.BatchMesh(spriteA.GetMaterial(), meshContainer, { 0, 1, 2, 0, 2, 3 });

			LoadQuad(meshContainer, glm::vec3(0.35f, 0.f, 0.f), wgpu::ColorF(0.f, 0.f, 1.f));
			renderer.BatchMesh(spriteB.GetMaterial(), meshContainer, { 0, 1, 2, 0, 2, 3 });

			LoadQuad(meshContainer, glm::vec3(0.f, 0.35f, 0.f), wgpu::ColorF(0.f, 1.f, 1.f));
			renderer.BatchMesh(spriteA.GetMaterial(), meshContainer, { 0, 1, 2, 0, 2, 3 });

			LoadQuad(meshContainer, glm::vec3(0.f, -0.35f, 0.f), wgpu::ColorF(1.f, 0.f, 1.f));
			renderer.BatchMesh(spriteB.GetMaterial(), meshContainer, { 0, 1, 2, 0, 2, 3 });
		}
		renderer.EndFrame();
		renderer.Render();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}