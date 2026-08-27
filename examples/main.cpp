#include <iostream>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <Renderer2D.h>
#include <Camera2D.h>

#include <Shader.h>
#include <BindGroupLayout.h>
#include <Pipeline.h>
#include <Material.h>
#include <Texture2D.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>

static void LoadQuad(wgpu::Mesh3D& mesh, const glm::vec3 position, const wgpu::ColorF& color)
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

static void LoadStar(wgpu::Mesh3D& mesh, const glm::vec3 position, const wgpu::ColorF& color)
{
	mesh.vertices.resize(8, wgpu::Vertex3D{});

	mesh.vertices[0].position = glm::vec3(0.f, 0.25f, 0.f) + position;
	mesh.vertices[0].color = color;
	mesh.vertices[0].uv = glm::vec2(0.f, 0.f);
	
	mesh.vertices[1].position = glm::vec3(-0.10f, 0.10f, 0.f) + position;
	mesh.vertices[1].color = color;
	mesh.vertices[1].uv = glm::vec2(0.f, 1.f);
	
	mesh.vertices[2].position = glm::vec3(-0.25f, 0.f, 0.f) + position;
	mesh.vertices[2].color = color;
	mesh.vertices[2].uv = glm::vec2(1.f, 1.f);
	
	mesh.vertices[3].position = glm::vec3(-0.10f, -0.10f, 0.f) + position;
	mesh.vertices[3].color = color;
	mesh.vertices[3].uv = glm::vec2(1.f, 0.f);
	
	mesh.vertices[4].position = glm::vec3(0.f, -0.25f, 0.f) + position;
	mesh.vertices[4].color = color;
	mesh.vertices[4].uv = glm::vec2(1.f, 0.f);
	
	mesh.vertices[5].position = glm::vec3(0.10f, -0.10f, 0.f) + position;
	mesh.vertices[5].color = color;
	mesh.vertices[5].uv = glm::vec2(1.f, 0.f);
	
	mesh.vertices[6].position = glm::vec3(0.25f, 0.f, 0.f) + position;
	mesh.vertices[6].color = color;
	mesh.vertices[6].uv = glm::vec2(1.f, 0.f);
	
	mesh.vertices[7].position = glm::vec3(0.10f, 0.10f, 0.f) + position;
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
	auto camera = renderer.GetCamera();
	camera->SetAspectRatio(800.f / 600.f);

	wgpu::Texture2D sprite{ renderer, "resources/Sprite.png" };
	auto material = renderer.GetSolidColorMaterial();
	const int bindingColor = material->GetUniformBinding<wgpu::CameraData>();
	material->SetUniform<wgpu::CameraData>(bindingColor, camera->GetCameraData());

	const int bindingSprite = sprite.GetMaterial()->GetUniformBinding<wgpu::CameraData>();
	sprite.GetMaterial()->SetUniform<wgpu::CameraData>(bindingColor, camera->GetCameraData());

	wgpu::Shader ppShader{ renderer, "resources/PostInverse.wgsl" };
	wgpu::BindGroupLayout ppLayout{};
	ppLayout.AddFrameEntry(0);
	ppLayout.ConfirmLayout(renderer);

	wgpu::Pipeline ppPipeline{ ppShader, wgpu::Pipeline::Type::PostProcessing, &ppLayout };
	wgpu::Material ppMaterial{ ppPipeline };

	wgpu::Mesh3D mesh{};
	bool invertColors{ false };

	// main loop
	bool isRunning{ true };
	while (isRunning)
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);

			ImGuiIO& io = ImGui::GetIO();

			if (io.WantCaptureMouse) continue;
			if (io.WantCaptureKeyboard) continue;

			isRunning = event.type != SDL_EVENT_QUIT;

			switch (event.type)
			{
			case SDL_EVENT_WINDOW_RESIZED:
				renderer.Resize();
				material->SetUniform<wgpu::CameraData>(bindingColor, camera->GetCameraData());
				sprite.GetMaterial()->SetUniform<wgpu::CameraData>(bindingSprite, camera->GetCameraData());
				break;

			case SDL_EVENT_MOUSE_MOTION:
				if (event.motion.state & SDL_BUTTON_LEFT)
				{
					const glm::vec2 motion{ -event.motion.xrel, event.motion.yrel };

					camera->Move(motion / 1000.f);
				}
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
			LoadStar(mesh, pos, wgpu::ColorF(0.f, 1.f, 0.f, 0.5f));
			renderer.BatchMesh(material, mesh);
			
			LoadQuad(mesh, glm::vec3(-0.5f, 0.f, 0.f), wgpu::ColorF(0.f, 1.f, 0.f));
			renderer.BatchMesh(sprite.GetMaterial(), mesh);

			angle += pi / 2.f;
			pos = glm::vec3(scale * cosf(angle), scale * sinf(angle), 1.f);
			LoadStar(mesh, pos, wgpu::ColorF(0.f, 0.f, 1.f, 0.5f));
			renderer.BatchMesh(material, mesh);
			
			LoadQuad(mesh, glm::vec3(0.5f, 0.f, 0.f), wgpu::ColorF(0.f, 0.f, 1.f));
			renderer.BatchMesh(sprite.GetMaterial(), mesh);

			angle += pi / 2.f;
			pos = glm::vec3(scale * cosf(angle), scale * sinf(angle), 1.f);
			LoadStar(mesh, pos, wgpu::ColorF(0.f, 1.f, 1.f, 0.5f));
			renderer.BatchMesh(material, mesh);

			LoadQuad(mesh, glm::vec3(0.f, 0.5f, 0.f), wgpu::ColorF(0.f, 1.f, 1.f));
			renderer.BatchMesh(sprite.GetMaterial(), mesh);

			angle += pi / 2.f;
			pos = glm::vec3(scale * cosf(angle), scale * sinf(angle), 1.f);
			LoadStar(mesh, pos, wgpu::ColorF(1.f, 0.f, 1.f, 0.5f));
			renderer.BatchMesh(material, mesh);

			LoadQuad(mesh, glm::vec3(0.f, -0.5f, 0.f), wgpu::ColorF(1.f, 0.f, 1.f));
			renderer.BatchMesh(sprite.GetMaterial(), mesh);
		}
		renderer.EndFrame();

		renderer.GuiBeginFrame();
		{
			ImGui::Begin("Renderer");
			{
				if (ImGui::RadioButton("Post Processing Effects", invertColors))
				{
					invertColors = !invertColors;
				}
			}
			ImGui::End();
		}
		renderer.GuiEndFrame();
		
		// just to show that multiple effects can be layered
		if (invertColors)
		{
			renderer.SubmitPostProcessingEffect(&ppMaterial);
			renderer.SubmitPostProcessingEffect(&ppMaterial);
			renderer.SubmitPostProcessingEffect(&ppMaterial);
			renderer.SubmitPostProcessingEffect(&ppMaterial);
			renderer.SubmitPostProcessingEffect(&ppMaterial);
		}
		
		renderer.Render();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}