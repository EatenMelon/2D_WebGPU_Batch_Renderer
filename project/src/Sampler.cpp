#include "Sampler.h"

wgpu::Sampler::Sampler(const GraphicsContext& context, Preset preset)
	: Sampler(context, GetSettings(preset))
{}

wgpu::Sampler::Sampler(const GraphicsContext& context, AddressMode u, AddressMode v, FilterMode mag, FilterMode min)
	: m_Context{ &context }
{
	WGPUSamplerDescriptor samplerDesc{};
	samplerDesc.addressModeU = GetWGPUAddressMode(u);
	samplerDesc.addressModeV = GetWGPUAddressMode(v);

	samplerDesc.addressModeW = WGPUAddressMode_ClampToEdge;

	samplerDesc.magFilter = GetFilterMode(mag);
	samplerDesc.minFilter = GetFilterMode(min);

	samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
	samplerDesc.lodMinClamp = 0.0f;
	samplerDesc.lodMaxClamp = 0.0f;

	samplerDesc.compare = WGPUCompareFunction_Undefined;
	samplerDesc.maxAnisotropy = 1;

	m_Sampler = wgpuDeviceCreateSampler(m_Context->GetDevice(), &samplerDesc);
}

wgpu::Sampler::~Sampler() noexcept
{
	wgpuSamplerRelease(m_Sampler);
}

wgpu::Sampler::Sampler(const GraphicsContext& context, SamplerSettings settings)
	: Sampler(context, settings.addressModeU, settings.addressModeV, settings.magFilter, settings.minFilter)
{}

wgpu::Sampler::SamplerSettings wgpu::Sampler::GetSettings(Preset preset)
{
	SamplerSettings settings{};

	settings.addressModeU = AddressMode::Repeat;
	settings.addressModeV = AddressMode::Repeat;

	switch (preset)
	{
	case wgpu::Sampler::Preset::Nearest:
		settings.magFilter = FilterMode::Nearest;
		settings.minFilter = FilterMode::Nearest;
		break;

	default:
		settings.magFilter = FilterMode::Linear;
		settings.minFilter = FilterMode::Linear;
		break;
	}

	return settings;
}

WGPUAddressMode wgpu::Sampler::GetWGPUAddressMode(AddressMode mode)
{
	switch (mode)
	{
	case wgpu::AddressMode::MirrorRepeat:	return WGPUAddressMode_MirrorRepeat;
	case wgpu::AddressMode::Clamp:			return WGPUAddressMode_ClampToEdge;
	default:								return WGPUAddressMode_Repeat;
	}
}

WGPUFilterMode wgpu::Sampler::GetFilterMode(FilterMode mode)
{
	switch (mode)
	{
	case wgpu::FilterMode::Linear:	return WGPUFilterMode_Linear;
	default:						return WGPUFilterMode_Nearest;
	}
}
