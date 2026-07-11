#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "Renderer2D.h"
#include "Pipeline.h"

std::unique_ptr<wgpu::Shader> g_Shader{ nullptr };
std::unique_ptr<wgpu::Pipeline> g_Pipeline{ nullptr };

//static void TestRenderCallback(WGPURenderPassEncoder renderPass)
//{
//	// Select which render pipeline to use
//	wgpuRenderPassEncoderSetPipeline(renderPass, g_Pipeline->GetPipeline());
//	// Draw 1 instance of a 3-vertices shape
//	wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);
//}

static void InitRenderResources(const wgpu::Renderer2D& renderer)
{
	std::filesystem::path path{ "resources/TestShader.wgsl" };

	g_Shader = std::make_unique<wgpu::Shader>(*renderer.GetContext(), path);
	g_Pipeline = std::make_unique<wgpu::Pipeline>(*g_Shader.get());
}

int main()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	glm::ivec2 size{ 800, 600 };
	SDL_Window* window = SDL_CreateWindow("Hello WebGPU", size.x, size.y, NULL);

	wgpu::Renderer2D renderer{};
	if (!renderer.Init(window)) return -1;

	InitRenderResources(renderer);

	renderer.SetClearColor(0.f, 0.f, 0.1f);

	bool isRunning{ true };
	while (isRunning)
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			isRunning = event.type != SDL_EVENT_QUIT;
		}

		renderer.Render();
	}
	renderer.Quit();

	SDL_DestroyWindow(window);
	SDL_Quit();
}