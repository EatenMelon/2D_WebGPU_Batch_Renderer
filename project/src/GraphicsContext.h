#ifndef _GRAPHICS_CONTEXT
#define _GRAPHICS_CONTEXT

#include <webgpu/webgpu.h>
#include <glm/glm.hpp>

struct SDL_Window;

namespace wgpu
{
	class GraphicsContext final
	{
	public:
		GraphicsContext(SDL_Window* window);
		~GraphicsContext();

		GraphicsContext(const GraphicsContext&) = delete;
		GraphicsContext& operator=(const GraphicsContext&) = delete;
		GraphicsContext(GraphicsContext&&) = delete;
		GraphicsContext& operator=(GraphicsContext&&) = delete; 

		bool InitSurface();
		void DestroySurface();

		WGPUInstance GetInstance() const { return m_Instance; }
		WGPUAdapter GetAdapter() const { return m_Adapter; }
		WGPUDevice GetDevice() const { return m_Device; }
		WGPUQueue GetQueue() const { return m_Queue; }
		WGPUSurface GetSurface() const { return m_Surface; }
		WGPUTextureFormat GetSurfaceFormat() const { return m_SurfaceFormat; }

		SDL_Window* GetWindow() const { return m_Window; }
		glm::vec2 GetWindowSize() const { return m_WindowSize; }
		float GetAspectRatio() const;

	private:
		bool CreateInstance();
		bool RequestAdapter();
		bool RequestDevice();
		bool RequestQueue();

		WGPUInstance m_Instance{ nullptr };
		WGPUAdapter m_Adapter{ nullptr };
		WGPUDevice m_Device{ nullptr };
		WGPUQueue m_Queue{ nullptr };
		WGPUSurface m_Surface{ nullptr };
		WGPUTextureFormat m_SurfaceFormat{ WGPUTextureFormat_Undefined };

		SDL_Window* m_Window{ nullptr };
		glm::vec2 m_WindowSize{};
	};
}

#endif
