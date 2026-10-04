#include <iostream>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <Renderer2D.h>
#include <Camera2D.h>

#include <Shader.h>
#include <BindGroupLayout.h>
#include <Pipeline.h>
#include <Material.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static void LoadQuad(wgpu::Mesh3D& mesh, const glm::vec3& position, const wgpu::ColorF& color)
{
	mesh.vertices.resize(4, wgpu::Vertex3D{});

	mesh.vertices[0].position = glm::vec3(-0.25f, 0.25f, 0.f) + position;
	mesh.vertices[0].color = color;
	mesh.vertices[0].uv = glm::vec2(0.f, 0.f);

	mesh.vertices[1].position = glm::vec3(-0.25f, -0.25f, 0.f) + position;
	mesh.vertices[1].color = color;
	mesh.vertices[1].uv = glm::vec2(0.f, 1.f);

	mesh.vertices[2].position = glm::vec3(0.25f, -0.25f, 0.f) + position;
	mesh.vertices[2].color = color;
	mesh.vertices[2].uv = glm::vec2(1.f, 1.f);

	mesh.vertices[3].position = glm::vec3(0.25f, 0.25f, 0.f) + position;
	mesh.vertices[3].color = color;
	mesh.vertices[3].uv = glm::vec2(1.f, 0.f);

	mesh.indices = { 0, 1, 2, 0, 2, 3 };
}

static void LoadStar(wgpu::Mesh3D& mesh, const glm::vec3& position, const wgpu::ColorF& color, float scale = 1.f)
{
	mesh.vertices.resize(8, wgpu::Vertex3D{});

	glm::mat4 transform(1.f);
	transform = glm::translate(transform, position);
	transform = glm::scale(transform, glm::vec3(scale, scale, 1.f));

	mesh.vertices[0].position = transform * glm::vec4(0.f, 0.25f, 0.f, 1.f);
	mesh.vertices[0].color = color;
	mesh.vertices[0].uv = glm::vec2(0.f, 0.f);
	
	mesh.vertices[1].position = transform * glm::vec4(-0.10f, 0.10f, 0.f, 1.f);
	mesh.vertices[1].color = color;
	mesh.vertices[1].uv = glm::vec2(0.f, 1.f);
	
	mesh.vertices[2].position = transform * glm::vec4(-0.25f, 0.f, 0.f, 1.f);
	mesh.vertices[2].color = color;
	mesh.vertices[2].uv = glm::vec2(1.f, 1.f);
	
	mesh.vertices[3].position = transform * glm::vec4(-0.10f, -0.10f, 0.f, 1.f);
	mesh.vertices[3].color = color;
	mesh.vertices[3].uv = glm::vec2(1.f, 0.f);
	
	mesh.vertices[4].position = transform * glm::vec4(0.f, -0.25f, 0.f, 1.f);
	mesh.vertices[4].color = color;
	mesh.vertices[4].uv = glm::vec2(1.f, 0.f);
	
	mesh.vertices[5].position = transform * glm::vec4(0.10f, -0.10f, 0.f, 1.f);
	mesh.vertices[5].color = color;
	mesh.vertices[5].uv = glm::vec2(1.f, 0.f);
	
	mesh.vertices[6].position = transform * glm::vec4(0.25f, 0.f, 0.f, 1.f);
	mesh.vertices[6].color = color;
	mesh.vertices[6].uv = glm::vec2(1.f, 0.f);
	
	mesh.vertices[7].position = transform * glm::vec4(0.10f, 0.10f, 0.f, 1.f);
	mesh.vertices[7].color = color;
	mesh.vertices[7].uv = glm::vec2(1.f, 0.f);
	
	mesh.indices.clear();
	
	mesh.indices.push_back(0);
	mesh.indices.push_back(1);
	mesh.indices.push_back(7);
	
	mesh.indices.push_back(1);
	mesh.indices.push_back(2);
	mesh.indices.push_back(3);
	
	mesh.indices.push_back(3);
	mesh.indices.push_back(4);
	mesh.indices.push_back(5);
	
	mesh.indices.push_back(5);
	mesh.indices.push_back(6);
	mesh.indices.push_back(7);
	
	mesh.indices.push_back(1);
	mesh.indices.push_back(3);
	mesh.indices.push_back(5);
	
	mesh.indices.push_back(1);
	mesh.indices.push_back(5);
	mesh.indices.push_back(7);
}

int main()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	
	glm::ivec2 size{ 800, 600 };
	SDL_Window* window = SDL_CreateWindow("Welcome back, WebGPU", size.x, size.y, SDL_WINDOW_RESIZABLE);

	wgpu::Renderer2D renderer{ window };
	
	// render resources
	wgpu::Camera2D camera{};
	camera.SetAspectRatio(static_cast<float>(size.x) / size.y);

	wgpu::BindGroupLayout layout{};
	layout.SetUniformCount(2);
	layout.AddUniformVariable<wgpu::CameraData>(0);
	layout.AddUniformVariable<wgpu::ColorF>(1);
	layout.ConfirmLayout(renderer);

	wgpu::Shader shader{ renderer, "resources/SolidColor.wgsl" };
	wgpu::Pipeline pipeline{ shader, wgpu::Pipeline::Type::GeometryTransparent, &layout };
	wgpu::Material material{ pipeline };

	material.SetUniformVariable<wgpu::ColorF>(1, wgpu::ColorF{ 1.f, 1.f, 1.f, 0.5f });
	material.SetUniformVariable<wgpu::CameraData>(0, camera.GetCameraData());

	wgpu::Mesh3D mesh{};

	constexpr auto getRandomFloat = [](float min, float max) -> float
		{
			constexpr int precision = 1000;
			int minInt = static_cast<int>(min * precision);
			int maxInt = static_cast<int>(max * precision);

			int randInt = std::rand() % (maxInt - minInt) + minInt;

			return static_cast<float>(randInt) / precision;
		};

	std::srand(std::time(0));
	constexpr int numStars{ 1'000 };

	std::vector<glm::vec3> starPositions(numStars);
	for (auto& pos : starPositions)
	{
		pos.x = getRandomFloat(-0.5f, 0.5f);
		pos.y = getRandomFloat(-0.5f, 0.5f);
		pos.z = getRandomFloat(-1.f, 1.f);
	}

	std::vector<wgpu::ColorF> starColors(numStars);
	for (auto& color : starColors)
	{
		color.r = getRandomFloat(0.1f, 1.f);
		color.g = getRandomFloat(0.1f, 1.f);
		color.b = getRandomFloat(0.1f, 1.f);
	}

	// main loop
	bool isRunning{ true };
	while (isRunning)
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			isRunning = event.type != SDL_EVENT_QUIT;

			bool updateMaterials{ false };
			switch (event.type)
			{
			case SDL_EVENT_WINDOW_RESIZED:
				renderer.Resize();
				camera.SetAspectRatio(static_cast<float>(event.window.data1) / event.window.data2);
				updateMaterials = true;
				break;

			case SDL_EVENT_MOUSE_MOTION:
				if (!(event.motion.state & SDL_BUTTON_LEFT)) break;
				
				const glm::vec2 motion{ -event.motion.xrel, event.motion.yrel };
				camera.Move(motion / 1000.f);
				updateMaterials = true;
				
				break;
			}

			if (updateMaterials)
			{
				material.SetUniformVariable<wgpu::CameraData>(0, camera.GetCameraData());
			}
		}
		
		renderer.BeginFrame();
		{
			for (size_t idx{0}; idx < numStars; ++idx)
			{
				const auto& pos = starPositions[idx];
				const auto& color = starColors[idx];

				LoadStar(mesh, pos, color, 0.1f);
				renderer.BatchMesh(&material, mesh);
			}
		}
		renderer.EndFrame();
		
		renderer.Render();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}