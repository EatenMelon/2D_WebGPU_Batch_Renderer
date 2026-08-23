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
	auto camera{ std::make_shared<wgpu::Camera2D>() };

	const wgpu::ColorF darkBlue{ 0.f, 0.f, 0.35f };
	const wgpu::ColorF red{ 1.f, 0.f, 0.f };
	const wgpu::ColorF green{ 0.f, 1.f, 0.f };
	const wgpu::ColorF lessDarkBlue{ 0.f, 0.f, 0.5f };

	canvas.SetCamera(camera);
	canvas.SetClearColor(darkBlue);

	wgpu::Texture2D sprite{ canvas, "resources/Sprite.png" };

	bool isRunning{ true };
	while (isRunning)
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			isRunning = event.type != SDL_EVENT_QUIT;

			switch (event.type)
			{
			case SDL_EVENT_WINDOW_RESIZED:	canvas.Resize();						break;
			case SDL_EVENT_MOUSE_WHEEL:		camera->Zoom(event.wheel.y / 100.f);	break;

			case SDL_EVENT_MOUSE_MOTION:
				if ((event.motion.state & SDL_BUTTON_LEFT) != SDL_BUTTON_LEFT)		break;

				glm::vec2 motion{ -event.motion.xrel, event.motion.yrel };
				camera->Move(motion / 1000.f * camera->GetZoom());
				break;
			}
		}

		canvas.BeginFrame();
		{
			constexpr float lineWidth{ 0.0075f };

			const wgpu::RectF rectangle{ -0.625f, -0.25f, 0.25f, 0.5f };
			canvas.SetDrawLayer(0.f);
			canvas.SetDrawColor(green);
			canvas.FillRect(rectangle);
			canvas.SetDrawColor(red);
			canvas.DrawRect(rectangle, lineWidth);

			const wgpu::EllipseF ellipse{ 0.5f, 0.f, 0.125f, 0.25f };
			canvas.SetDrawColor(green);
			canvas.FillEllipse(ellipse);
			canvas.SetDrawColor(red);
			canvas.DrawEllipse(ellipse, lineWidth);

			const wgpu::RectF spriteFrame1{ -0.25f, -0.25f, 0.5f, 0.5f };
			const wgpu::RectF spriteFrame2{ -0.f, -0.f, 0.5f, 0.5f };
			canvas.SetDrawColor(lessDarkBlue);
			canvas.FillRect(spriteFrame1);
			canvas.DrawRect(spriteFrame1, 0.05f);

			// order doesn't matter here
			canvas.SetDrawLayer(2.f);
			canvas.DrawTexture(sprite, spriteFrame2);
			canvas.SetDrawLayer(1.f);
			canvas.SetDrawColor(wgpu::ColorF{ 1.f, 1.f, 1.f });
			canvas.RenderRect(*sprite.GetMaterial(), spriteFrame1);
			// now it does matter again
			
			canvas.SetDrawLayer(0.f);
			const glm::vec2 start{ -0.5f, -0.33f };
			const glm::vec2 end{ 0.5f, -0.33f };
			canvas.SetDrawColor(green);
			canvas.DrawLine(start, end, lineWidth);
			canvas.DrawLine(-start, -end, lineWidth);

			canvas.SetDrawLayer(4.f);

			wgpu::Vertex2D v0{};
			v0.position = glm::vec2{ -0.5f, -0.5f };
			v0.uv = glm::vec2{ 1.f, 1.f };
			v0.color = wgpu::ColorF{ 0.f, 1.f, 0.f };

			wgpu::Vertex2D v1{};
			v1.position = glm::vec2{ 0.5f, -0.5f };
			v1.uv = glm::vec2{ 0.f, 1.f };
			v1.color = wgpu::ColorF{ 0.f, 0.f, 1.f, 0.5f };

			wgpu::Vertex2D v2{};
			v2.position = glm::vec2{ 0.f, 0.5f };
			v2.uv = glm::vec2{ 0.5f, 0.f };
			v2.color = wgpu::ColorF{ 1.f, 0.f, 0.f };

			canvas.RenderTriangle(*sprite.GetMaterial(), v0, v1, v2);
			canvas.FillTriangle(v0.position, v1.position, v2.position);
		}
		canvas.EndFrame();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}