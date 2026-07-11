#ifndef _RENDERER_2D
#define _RENDERER_2D

#include <memory>
#include <utility>

#include "GraphicsContext.h"
#include "RenderQueue.h"

struct SDL_Window;

namespace wgpu
{
	class Renderer2D final
	{
	public:
		bool Init(SDL_Window* window);

		void BeginFrame();
		void EndFrame();
		void Render() const;

		void Quit();

		void SetClearColor(float r, float g, float b, float a = 1.f);

		WGPUBuffer GetVertexBuffer() const { return m_VertexBuffer.buffer; }
		const GraphicsContext* GetContext() const { return m_Context.get(); }

	private:
		std::pair<WGPUSurfaceTexture, WGPUTextureView> GetNextSurfaceViewData() const;

		void CreateVertexBuffer(size_t capacity);

		std::unique_ptr<GraphicsContext> m_Context{ nullptr };
		std::unique_ptr<RenderQueue> m_RenderQueue{ nullptr };

		WGPUColor m_ClearColor{ 0.f, 0.f, 0.f, 1.f };

		struct VertexBuffer
		{
			WGPUBuffer buffer{ nullptr };
			size_t capacity{};
		};

		VertexBuffer m_VertexBuffer{};
	};
}

#endif
