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
	class BuiltinResources;

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
		void BatchMesh(Material* material, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices) const;
		void EndFrame();
		void Render() const;

		void SetClearColor(const ColorF& color);
		void SetCamera(const std::shared_ptr<Camera2D>& camera);
		void Resize();

		std::shared_ptr<Camera2D> GetCamera() const;

		WGPUTexture GetDepthTexture() const { return m_DepthTexture; }

		const GraphicsContext* GetContext() const { return m_Context.get(); }
		const BuiltinResources* GetBuiltinResources() const { return m_BuiltinResources.get(); }

	private:
		// additional render resources
		ColorF m_ClearColor{ 0.f, 0.f, 0.f, 1.f };
		std::shared_ptr<Camera2D> m_Camera{ nullptr };
		std::unique_ptr<BuiltinResources> m_BuiltinResources{ nullptr };

		// surface view
		std::pair<WGPUSurfaceTexture, WGPUTextureView> GetNextSurfaceViewData() const;

		// depth buffer
		void InitDepthBuffer();
		void ReleaseDepthBuffer();

		WGPUTexture m_DepthTexture{ nullptr };
		WGPUTextureView m_DepthTextureView{ nullptr };

		// buffers
		void CreateVertexBuffer(size_t capacity);
		void CreateIndexBuffer(size_t capacity);

		struct Buffer
		{
			WGPUBuffer buffer{ nullptr };
			size_t capacity{};
		};

		Buffer m_Vertex{};
		Buffer m_Index{};

		// render info
		std::unique_ptr<GraphicsContext> m_Context{ nullptr };

		// batch management
		std::unique_ptr<RenderQueue> m_RenderQueue{ nullptr };
	};
}

#endif
