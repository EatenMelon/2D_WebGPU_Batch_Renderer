#include "BindGroupLayout.h"

#include <vector>
#include <algorithm>
#include <stdexcept>

bool wgpu::BindGroupLayout::AddTextureEntry(int binding)
{
	if (IsLocked()) return false;

	WGPUBindGroupLayoutEntry entry{};
	entry.binding = binding;
	entry.visibility = WGPUShaderStage_Fragment;
	entry.texture.sampleType = WGPUTextureSampleType_Float;
	entry.texture.viewDimension = WGPUTextureViewDimension_2D;

	m_Entries.insert_or_assign(binding, entry);

	return true;
}

bool wgpu::BindGroupLayout::RequiresUniform() const
{
	return m_UniformEntry.has_value();
}

int wgpu::BindGroupLayout::GetUniformEntryBinding() const
{
	if (!m_UniformEntry.has_value())
	{
		throw std::runtime_error("You can't request a binding for a uniform that doesn't exist!");
	}

	return m_UniformEntry.value().second.binding;
}

void wgpu::BindGroupLayout::ConfirmLayout(const GraphicsContext& context)
{
	m_Context = &context;

	std::vector<WGPUBindGroupLayoutEntry> entries{};

	if (m_UniformEntry.has_value())
	{
		entries.push_back(m_UniformEntry.value().second);
	}

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
	if (!m_UniformEntry.has_value())
	{
		return 0;
	}

	return m_UniformEntry.value().second.buffer.minBindingSize;
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
