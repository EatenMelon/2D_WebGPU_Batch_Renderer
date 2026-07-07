#ifndef _RENDERER_2D
#define _RENDERER_2D

#include <memory>
#include <utility>

#include "GraphicsContext.h"

struct SDL_Window;

namespace wgpu
{
	class GraphicsContext;
	class Renderer2D final
	{
	public:
		Renderer2D() noexcept = default;

		bool Init(SDL_Window* window);
		void Render() const;
		void Quit();

		void SetClearColor(float r, float g, float b, float a = 1.f);

	private:
		std::pair<WGPUSurfaceTexture, WGPUTextureView> GetNextSurfaceViewData() const;

		std::unique_ptr<GraphicsContext> m_Context{ nullptr };

		WGPUColor m_ClearColor{ 0.f, 0.f, 0.f, 1.f };
	};
}

#endif
