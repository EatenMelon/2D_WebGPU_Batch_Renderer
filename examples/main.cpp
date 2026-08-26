#include <iostream>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <Renderer2D.h>
#include <Camera2D.h>
#include <Material.h>
#include <Texture2D.h>

static void LoadQuad(std::vector<wgpu::Vertex3D>& vertices, std::vector<uint32_t>& indices, const glm::vec3 position, const wgpu::ColorF& color)
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

	indices = { 0, 1, 2, 0, 2, 3 };
}

static void LoadStar(std::vector<wgpu::Vertex3D>& vertices, std::vector<uint32_t>& indices, const glm::vec3 position, const wgpu::ColorF& color)
{
	vertices.resize(8, wgpu::Vertex3D{});

	vertices[0].position = glm::vec3(0.f, 0.25f, 0.f) + position;
	vertices[0].color = color;
	vertices[0].uv = glm::vec2(0.f, 0.f);

	vertices[1].position = glm::vec3(-0.10f, 0.10f, 0.f) + position;
	vertices[1].color = color;
	vertices[1].uv = glm::vec2(0.f, 1.f);

	vertices[2].position = glm::vec3(-0.25f, 0.f, 0.f) + position;
	vertices[2].color = color;
	vertices[2].uv = glm::vec2(1.f, 1.f);

	vertices[3].position = glm::vec3(-0.10f, -0.10f, 0.f) + position;
	vertices[3].color = color;
	vertices[3].uv = glm::vec2(1.f, 0.f);

	vertices[4].position = glm::vec3(0.f, -0.25f, 0.f) + position;
	vertices[4].color = color;
	vertices[4].uv = glm::vec2(1.f, 0.f);

	vertices[5].position = glm::vec3(0.10f, -0.10f, 0.f) + position;
	vertices[5].color = color;
	vertices[5].uv = glm::vec2(1.f, 0.f);

	vertices[6].position = glm::vec3(0.25f, 0.f, 0.f) + position;
	vertices[6].color = color;
	vertices[6].uv = glm::vec2(1.f, 0.f);

	vertices[7].position = glm::vec3(0.10f, 0.10f, 0.f) + position;
	vertices[7].color = color;
	vertices[7].uv = glm::vec2(1.f, 0.f);

	indices.clear();

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(7);

	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);

	indices.push_back(3);
	indices.push_back(4);
	indices.push_back(5);

	indices.push_back(5);
	indices.push_back(6);
	indices.push_back(7);

	indices.push_back(1);
	indices.push_back(3);
	indices.push_back(5);

	indices.push_back(1);
	indices.push_back(5);
	indices.push_back(7);
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

	wgpu::Texture2D sprite{ renderer, "resources/Sprite.png" };
	auto material = renderer.GetSolidColorMaterial();
	const int bindingColor = material->GetUniformBinding<wgpu::CameraData>();
	material->SetUniform<wgpu::CameraData>(bindingColor, camera->GetCameraData());

	const int bindingSprite = sprite.GetMaterial()->GetUniformBinding<wgpu::CameraData>();
	sprite.GetMaterial()->SetUniform<wgpu::CameraData>(bindingColor, camera->GetCameraData());

	std::vector<wgpu::Vertex3D> vertices{};
	std::vector<uint32_t> indices{};

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
				material->SetUniform<wgpu::CameraData>(bindingColor, camera->GetCameraData());
				sprite.GetMaterial()->SetUniform<wgpu::CameraData>(bindingSprite, camera->GetCameraData());
				break;
			}
		}

		renderer.BeginFrame();
		{
			float time = SDL_GetTicks() / 1000.f;
			const float pi{ 3.14f };

			float scale{ 0.33f };
			float angle{ time };

			glm::vec3 pos{ scale * cosf(angle), scale * sinf(angle), 1.f };
			LoadStar(vertices, indices, pos, wgpu::ColorF(0.f, 1.f, 0.f, 0.5f));
			renderer.BatchMesh(material, vertices, indices);
			
			LoadQuad(vertices, indices, glm::vec3(-0.5f, 0.f, 0.f), wgpu::ColorF(0.f, 1.f, 0.f));
			renderer.BatchMesh(sprite.GetMaterial(), vertices, indices);

			angle += pi / 2.f;
			pos = glm::vec3(scale * cosf(angle), scale * sinf(angle), 1.f);
			LoadStar(vertices, indices, pos, wgpu::ColorF(0.f, 0.f, 1.f, 0.5f));
			renderer.BatchMesh(material, vertices, indices);
			
			LoadQuad(vertices, indices, glm::vec3(0.5f, 0.f, 0.f), wgpu::ColorF(0.f, 0.f, 1.f));
			renderer.BatchMesh(sprite.GetMaterial(), vertices, indices);

			angle += pi / 2.f;
			pos = glm::vec3(scale * cosf(angle), scale * sinf(angle), 1.f);
			LoadStar(vertices, indices, pos, wgpu::ColorF(0.f, 1.f, 1.f, 0.5f));
			renderer.BatchMesh(material, vertices, indices);

			LoadQuad(vertices, indices, glm::vec3(0.f, 0.5f, 0.f), wgpu::ColorF(0.f, 1.f, 1.f));
			renderer.BatchMesh(sprite.GetMaterial(), vertices, indices);

			angle += pi / 2.f;
			pos = glm::vec3(scale * cosf(angle), scale * sinf(angle), 1.f);
			LoadStar(vertices, indices, pos, wgpu::ColorF(1.f, 0.f, 1.f, 0.5f));
			renderer.BatchMesh(material, vertices, indices);

			LoadQuad(vertices, indices, glm::vec3(0.f, -0.5f, 0.f), wgpu::ColorF(1.f, 0.f, 1.f));
			renderer.BatchMesh(sprite.GetMaterial(), vertices, indices);
		}
		renderer.EndFrame();
		renderer.Render();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}