#ifndef _TEXTURE_2D
#define _TEXTURE_2D

#include <filesystem>
#include <memory>

#include <webgpu/webgpu.h>
#include "DataTypes.h"
#include "Sampler.h"

namespace wgpu
{
	class Renderer2D;
	class GraphicsContext;
	class Material;
	class Canvas;
	
	class Texture2D final
	{
	public:
		Texture2D(const Renderer2D& renderer, const std::filesystem::path& path);
		Texture2D(const Canvas& canvas, const std::filesystem::path& path);
		~Texture2D() noexcept;

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;
		Texture2D(Texture2D&&) = delete;
		Texture2D& operator=(Texture2D&&) = delete;

		void SetColorMultiplier(const ColorF& color);
		void SelectSampler(Sampler::Preset preset);

		RectF GetCutout(const RectF& src) const;

		glm::vec2 GetSize() const;
		ColorF GetColorMultiplier() const;
		Material* GetMaterial() const { return m_Material.get(); }

		WGPUTextureView GetView() const { return m_TextureView; }

	private:

		glm::vec2 m_Size{ 1.f, 1.f };
		ColorF m_ColorMultiplier{ 1.f, 1.f, 1.f };
		std::unique_ptr<Material> m_Material{ nullptr };

		WGPUTexture m_Texture{ nullptr };
		WGPUTextureView m_TextureView{ nullptr };

		const Renderer2D* m_Renderer{ nullptr };
	};
}

#endif
