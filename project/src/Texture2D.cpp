#include "Texture2D.h"
#include <SDL3_image/SDL_image.h>

#include "GraphicsContext.h"
#include "Renderer2D.h"
#include "Material.h"
#include "BuiltinResources.h"
#include "Canvas.h"

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

	m_Material = m_Renderer->GetBuiltinResources()->CreateTextureMaterial();
	m_Material->SetTexture(1, this);
	m_Material->SetSampler(2, m_Renderer->GetBuiltinResources()->GetLinearSampler());
}

wgpu::Texture2D::Texture2D(const Canvas& canvas, const std::filesystem::path& path)
	: Texture2D(*canvas.GetRenderer(), path)
{}

wgpu::Texture2D::~Texture2D() noexcept
{
	m_Material.reset();

	wgpuTextureViewRelease(m_TextureView);
	wgpuTextureDestroy(m_Texture);
	wgpuTextureRelease(m_Texture);
}

void wgpu::Texture2D::SetColorMultiplier(const ColorF& color)
{
	m_ColorMultiplier = color;
}

void wgpu::Texture2D::SelectSampler(Sampler::Preset preset)
{
	auto sampler{ m_Renderer->GetBuiltinResources()->GetLinearSampler() };

	switch (preset)
	{
	case Sampler::Preset::Nearest:
		sampler = m_Renderer->GetBuiltinResources()->GetNearestSampler();
		break;

	default:
		break;
	}

	m_Material->SetSampler(2, sampler);
}

wgpu::RectF wgpu::Texture2D::GetCutout(const RectF& src) const
{
	return RectF{ src.pos / m_Size, src.size / m_Size };
}

glm::vec2 wgpu::Texture2D::GetSize() const
{
	return m_Size;
}

wgpu::ColorF wgpu::Texture2D::GetColorMultiplier() const
{
	return m_ColorMultiplier;
}

