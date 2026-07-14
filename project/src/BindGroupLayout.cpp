#include "BindGroupLayout.h"

#include <vector>
#include <algorithm>

void wgpu::BindGroupLayout::ConfirmLayout(const GraphicsContext& context)
{
	m_Context = &context;

	std::vector<WGPUBindGroupLayoutEntry> entries{};

	for (const auto& [binding, entry] : m_Entries)
	{
		entries.push_back(entry);
	}

	std::sort
	(
		entries.begin(),
		entries.end(),
		[](const WGPUBindGroupLayoutEntry& a, const WGPUBindGroupLayoutEntry& b)
		{
			return a.binding < b.binding;
		}
	);

	WGPUBindGroupLayoutDescriptor desc{};
	desc.nextInChain = nullptr;
	desc.entryCount = entries.size();
	desc.entries = entries.data();

	m_BindGroupLayout = wgpuDeviceCreateBindGroupLayout(context.GetDevice(), &desc);
}

uint64_t wgpu::BindGroupLayout::GetRequiredUniformBufferSize() const
{
	uint64_t size{ 0 };

	for (const auto& [binding, entry] : m_Entries)
	{
		if (entry.buffer.type != WGPUBufferBindingType_Uniform)
		{
			continue;
		}

		size += entry.buffer.minBindingSize;
	}

	return size;
}

WGPUShaderStage wgpu::BindGroupLayout::GetShaderStage(BindingVisibility visibility)
{
	switch (visibility)
	{
	case wgpu::BindingVisibility::VertexShaderStage:
		return WGPUShaderStage_Fragment;

	case wgpu::BindingVisibility::FragmentShaderStage:
		return WGPUShaderStage_Fragment;

	case wgpu::BindingVisibility::Both:
		return WGPUShaderStage_Fragment | WGPUShaderStage_Fragment;

	default: break;
	}

	return WGPUShaderStage_None;
}
