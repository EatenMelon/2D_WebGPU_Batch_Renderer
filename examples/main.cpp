#include <iostream>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <Renderer2D.h>
#include <Camera2D.h>

int main()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	
	glm::ivec2 size{ 800, 600 };
	SDL_Window* window = SDL_CreateWindow("Hello WebGPU", size.x, size.y, SDL_WINDOW_RESIZABLE);

	wgpu::Renderer2D renderer{ window };
	renderer.SetClearColor(wgpu::ColorF{ 1.f, 1.f, 1.f });
	
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
			}
		}

		renderer.BeginFrame();
		{
			
		}
		renderer.EndFrame();
		renderer.Render();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}