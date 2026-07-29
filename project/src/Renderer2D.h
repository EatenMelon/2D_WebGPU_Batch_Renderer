#ifndef _RENDERER_2D
#define _RENDERER_2D

#include <memory>
#include <utility>

#include "GraphicsContext.h"
#include "RenderQueue.h"

#include "Camera2D.h"

struct SDL_Window;

namespace wgpu
{
	class Renderer2D final
	{
	public:
		bool Init(SDL_Window* window);

		void BeginFrame();
		void Submit(Material& mat, const Vertex& v0, const Vertex& v1, const Vertex& v2);
		void EndFrame();
		void Render() const;

		void Quit();

		void SetClearColor(float r, float g, float b, float a = 1.f);
		void SetCamera(const std::shared_ptr<Camera2D>& camera);
		void Resize();

		WGPUBuffer GetVertexBuffer() const { return m_VertexBuffer.buffer; }
		WGPUTexture GetDepthTexture() const { return m_DepthTexture; }

		const GraphicsContext* GetContext() const { return m_Context.get(); }

	private:
		ColorF m_ClearColor{ 0.f, 0.f, 0.f, 1.f };
		std::shared_ptr<Camera2D> m_Camera{ nullptr };

		std::pair<WGPUSurfaceTexture, WGPUTextureView> GetNextSurfaceViewData() const;

		void CreateVertexBuffer(size_t capacity);
		void InitDepthBuffer();
		void ReleaseDepthBuffer();

		std::unique_ptr<GraphicsContext> m_Context{ nullptr };
		std::unique_ptr<RenderQueue> m_RenderQueue{ nullptr };

		struct VertexBuffer
		{
			WGPUBuffer buffer{ nullptr };
			size_t capacity{};
		};

		VertexBuffer m_VertexBuffer{};
		WGPUTexture m_DepthTexture{ nullptr };
		WGPUTextureView m_DepthTextureView{ nullptr };
	};
}

#endif
