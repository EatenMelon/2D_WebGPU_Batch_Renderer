#include "BindGroupLayout.h"

void wgpu::BindGroupLayout::ClearEntries()
{
	m_Entries.clear();
}

void wgpu::BindGroupLayout::RemoveEntry(int binding)
{
	if (!m_Entries.contains(binding)) return;

	m_Entries.erase(binding);
}

WGPUShaderStage wgpu::BindGroupLayout::GetShaderStage(BindingVisibility visibility)
{
	switch (visibility)
	{
	case wgpu::BindingVisibility::VertexShaderStage:
		return WGPUShaderStage_Fragment;
		break;

	case wgpu::BindingVisibility::FragmentShaderStage:
		return WGPUShaderStage_Fragment;
		break;

	case wgpu::BindingVisibility::Both:
		return WGPUShaderStage_Fragment | WGPUShaderStage_Fragment;
		break;

	default:
		break;
	}

	return WGPUShaderStage_None;
}
