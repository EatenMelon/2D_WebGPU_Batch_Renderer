#include <iostream>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "Canvas.h"
#include "Texture2D.h"
#include "Camera2D.h"

int main()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	glm::ivec2 size{ 800, 600 };
	SDL_Window* window = SDL_CreateWindow("Hello WebGPU", size.x, size.y, SDL_WINDOW_RESIZABLE);

	wgpu::Canvas canvas{ window };
	wgpu::Texture2D texture{ canvas, "resources/texture.png" };
	texture.SelectSampler(wgpu::Sampler::Preset::Linear);

	auto camera{ std::make_shared<wgpu::Camera2D>() };

	const wgpu::ColorF darkBlue{ 0.f, 0.f, 0.1f };
	const wgpu::ColorF red{ 1.f, 0.f, 0.f };
	const wgpu::ColorF green{ 0.f, 1.f, 0.f };
	const wgpu::ColorF blue{ 0.f, 0.f, 1.f };
	const wgpu::ColorF violet{ 1.f, 0.f, 1.f };

	canvas.SetCamera(camera);
	canvas.SetClearColor(darkBlue);

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
				canvas.Resize();
				break;

			case SDL_EVENT_MOUSE_WHEEL:
				camera->Zoom(event.wheel.y / 100.f);
				break;

			case SDL_EVENT_MOUSE_MOTION:
				if ((event.motion.state & SDL_BUTTON_LEFT) != SDL_BUTTON_LEFT) break;

				glm::vec2 motion{ -event.motion.xrel, event.motion.yrel };
				camera->Move(motion / 1000.f * camera->GetZoom());

				break;

			default:
				break;
			}
		}

		canvas.BeginFrame();
		{
			canvas.SetDrawLayer(0.f);
			canvas.SetDrawColor(red);
			canvas.FillRect(-0.25f, -0.25f, 0.5f, 0.5f);

			canvas.SetDrawLayer(1.f);
			canvas.SetDrawColor(blue);
			canvas.FillRect(-0.125f, -0.125f, 0.25f, 0.25f);

			canvas.SetDrawLayer(2.f);
			canvas.SetDrawColor(green);
			canvas.DrawRect(-0.125f, -0.125f, 0.25f, 0.25f, 0.025f);

			canvas.SetDrawLayer(1.f);
			canvas.SetDrawColor(violet);
			canvas.DrawLine(glm::vec2{ -0.25f, -0.25f }, glm::vec2{ 0.25f, 0.25f });
			canvas.DrawLine(glm::vec2{ -0.25f, 0.25f }, glm::vec2{ 0.25f, -0.25f });

			canvas.SetDrawLayer(3.f);
			canvas.FillEllipse(0, 0, 0.125f / 1.5f, 0.125f / 2.5f);

			canvas.SetDrawLayer(4.f);
			canvas.SetDrawColor(green);
			canvas.DrawEllipse(0, 0, 0.125f / 1.5f, 0.125f / 2.5f, 0.005f);

			canvas.SetDrawLayer(3.f);
			canvas.DrawTexture
			(
				texture,
				wgpu::RectF{ -0.125f, -0.125f, 0.25f, 0.25f },
				wgpu::RectF{ 512.f * 2, 512.f * 2, 512.f * 2, 512.f * 2 }
			);
		}
		canvas.EndFrame();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}