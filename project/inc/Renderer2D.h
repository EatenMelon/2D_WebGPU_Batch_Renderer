#ifndef _RENDERER_2D
#define _RENDERER_2D

#include <memory>
#include <utility>
#include <wgpu.h>

#include "Camera2D.h"
#include <DataTypes.h>

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

		void BatchMesh(Material* material, const Mesh3D& mesh) const;
		void SubmitPostProcessingEffect(Material* material);

		void BeginFrame();
		void EndFrame();

		void GuiBeginFrame();
		void GuiEndFrame();

		void Render() const;

		void SetClearColor(const ColorF& color);
		void Resize();

		WGPUTexture GetDepthTexture() const { return m_DepthTexture; }

		const GraphicsContext* GetContext() const { return m_Context.get(); }

	private:
		void ImGuiInit();
		void ImGuiQuit();

		void InitDepthBuffer();
		void ReleaseDepthBuffer();

		void InitPostProcessingData();
		void DestroyPostProcessingData();

		void CreateVertexBuffer(size_t capacity);
		void CreateIndexBuffer(size_t capacity);
		
		std::pair<WGPUSurfaceTexture, WGPUTextureView> GetNextSurfaceViewData() const;

		void RenderObjects(WGPUTextureView targetView, WGPUCommandEncoder encoder) const;
		void RenderPostEffect(Material* effect, WGPUTextureView targetView, WGPUCommandEncoder encoder) const;
		void RenderGui(WGPUTextureView targetView, WGPUCommandEncoder encoder) const;

		struct Buffer
		{
			WGPUBuffer buffer{ nullptr };
			size_t capacity{};
		};
		struct PostProcessingData
		{
			WGPUTextureView pingView{ nullptr };
			WGPUTextureView pongView{ nullptr };
			WGPUTexture pingTexture{ nullptr };
			WGPUTexture pongTexture{ nullptr };
		};

		ColorF m_ClearColor{ 0.f, 0.f, 0.f, 1.f };

		WGPUTexture m_DepthTexture{ nullptr };
		WGPUTextureView m_DepthTextureView{ nullptr };
		PostProcessingData m_PPData{};

		Buffer m_Vertex{};
		Buffer m_Index{};

		std::unique_ptr<RenderQueue> m_RenderQueue{ nullptr };
		std::vector<Material*> m_PostEffects{ nullptr };


		std::unique_ptr<GraphicsContext> m_Context{ nullptr };
	};
}

#endif
