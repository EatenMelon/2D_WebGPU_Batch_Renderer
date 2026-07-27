#include "Texture2D.h"
#include <SDL3_image/SDL_image.h>

wgpu::Texture2D::Texture2D(const GraphicsContext& context, const std::filesystem::path& path)
	: m_Context{ &context }
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

	WGPUTextureDescriptor desc{};
	desc.dimension = WGPUTextureDimension_2D;
	desc.size.width = surface->w;
	desc.size.height = surface->h;
	desc.size.depthOrArrayLayers = 1;

	desc.format = WGPUTextureFormat_RGBA8Unorm;
	desc.mipLevelCount = 1;
	desc.sampleCount = 1;

	desc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;

	m_Texture = wgpuDeviceCreateTexture(m_Context->GetDevice(), &desc);

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

	WGPUQueue queue = wgpuDeviceGetQueue(m_Context->GetDevice());

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

wgpu::Texture2D::~Texture2D()
{
	wgpuTextureViewRelease(m_TextureView);
	wgpuTextureDestroy(m_Texture);
	wgpuTextureRelease(m_Texture);
}
