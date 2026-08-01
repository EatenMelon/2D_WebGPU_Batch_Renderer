#ifndef _RENDERER_2D
#define _RENDERER_2D

#include <memory>
#include <utility>
#include <wgpu.h>

#include "Camera2D.h"

struct SDL_Window;

namespace wgpu
{
	class RenderQueue;
	class GraphicsContext;
	class Material;
	class Renderer2D final
	{
	public:
		Renderer2D(SDL_Window* window);
		~Renderer2D() noexcept;

		Renderer2D(const Renderer2D&) = delete;
		Renderer2D& operator=(const Renderer2D&) = delete;
		Renderer2D(Renderer2D&&) = delete;
		Renderer2D& operator=(Renderer2D&&) = delete;

		void BeginFrame();
		void SubmitTriangle(Material& mat, const Vertex3D& v0, const Vertex3D& v1, const Vertex3D& v2);
		void SubmitQuad(Material& mat, const Vertex3D& v0, const Vertex3D& v1, const Vertex3D& v2, const Vertex3D& v3);
		void EndFrame();
		void Render() const;

		void SetClearColor(const ColorF& color);
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
