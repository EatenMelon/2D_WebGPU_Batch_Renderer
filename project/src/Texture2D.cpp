#include "Texture2D.h"
#include <SDL3_image/SDL_image.h>

#include "GraphicsContext.h"
#include "Renderer2D.h"
#include "Material.h"

wgpu::Texture2D::Texture2D(const Renderer2D& renderer, const std::filesystem::path& path)
	: m_Renderer{ &renderer }
{
	auto pathStr{ path.string() };
	auto surface = IMG_Load(pathStr.c_str());

	if (!surface)
	{
		throw std::runtime_error("Failed to load texture from file!");
	}
	else
	{
		auto converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
		SDL_DestroySurface(surface);

		surface = converted;
	}

	m_Size = glm::vec2{ surface->w, surface->h };

	WGPUTextureDescriptor desc{};
	desc.dimension = WGPUTextureDimension_2D;
	desc.size.width = surface->w;
	desc.size.height = surface->h;
	desc.size.depthOrArrayLayers = 1;

	desc.format = WGPUTextureFormat_RGBA8Unorm;
	desc.mipLevelCount = 1;
	desc.sampleCount = 1;

	desc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;

	m_Texture = wgpuDeviceCreateTexture(m_Renderer->GetContext()->GetDevice(), &desc);

	WGPUTexelCopyTextureInfo dst{};
	dst.texture = m_Texture;
	dst.mipLevel = 0;
	dst.origin = { 0, 0, 0 };

	WGPUTexelCopyBufferLayout src{};
	src.offset = 0;
	src.bytesPerRow = surface->pitch;
	src.rowsPerImage = surface->h;

	WGPUExtent3D size{};
	size.width = surface->w;
	size.height = surface->h;
	size.depthOrArrayLayers = 1;

	WGPUQueue queue = wgpuDeviceGetQueue(m_Renderer->GetContext()->GetDevice());

	wgpuQueueWriteTexture
	(
		queue,
		&dst,
		surface->pixels,
		surface->pitch * surface->h,
		&src,
		&size
	);

	m_TextureView = wgpuTextureCreateView(m_Texture, nullptr);
	SDL_DestroySurface(surface);
}

wgpu::Texture2D::~Texture2D() noexcept
{
	wgpuTextureViewRelease(m_TextureView);
	wgpuTextureDestroy(m_Texture);
	wgpuTextureRelease(m_Texture);
}

void wgpu::Texture2D::SetColorMultiplier(const ColorF& color)
{
	m_ColorMultiplier = color;
}

wgpu::RectF wgpu::Texture2D::GetCutout(const RectF& src) const
{
	return RectF
	{
		src.left / m_Size.x,
		src.bottom / m_Size.y,
		src.width / m_Size.x,
		src.height / m_Size.y
	};
}

glm::vec2 wgpu::Texture2D::GetSize() const
{
	return m_Size;
}

wgpu::ColorF wgpu::Texture2D::GetColorMultiplier() const
{
	return m_ColorMultiplier;
}

