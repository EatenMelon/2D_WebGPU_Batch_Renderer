#include "BindGroupLayout.h"

#include <vector>

void wgpu::BindGroupLayout::ConfirmLayout(const GraphicsContext& context)
{
	wgpuBindGroupLayoutRelease(m_BindGroupLayout);
	m_Context = &context;

	std::vector<WGPUBindGroupLayoutEntry> entries{};

	for (const auto& [binding, entry] : m_Entries)
	{
		entries.push_back(entry);
	}

	WGPUBindGroupLayoutDescriptor desc{};
	desc.nextInChain = nullptr;
	desc.entryCount = entries.size();
	desc.entries = entries.data();

	m_BindGroupLayout = wgpuDeviceCreateBindGroupLayout(context.GetDevice(), &desc);
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
