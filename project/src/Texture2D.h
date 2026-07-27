#ifndef _TEXTURE_2D
#define _TEXTURE_2D

#include <filesystem>

#include "GraphicsContext.h"

namespace wgpu
{
	class Texture2D final
	{
	public:
		Texture2D(const GraphicsContext& context, const std::filesystem::path& path);
		~Texture2D() noexcept;

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;
		Texture2D(Texture2D&&) = delete;
		Texture2D& operator=(Texture2D&&) = delete;

		WGPUTextureView GetView() const { return m_TextureView; }

	private:

		WGPUTexture m_Texture{ nullptr };
		WGPUTextureView m_TextureView{ nullptr };

		const GraphicsContext* m_Context{ nullptr };
	};
}

#endif
